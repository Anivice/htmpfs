#ifndef HTMPFS_PATH_T_H
#define HTMPFS_PATH_T_H

#include <string>
#include <vector>
#include <buffer_t.h>

class path_t;

/*
 *  _custom_it_t is a customized unidirectional iterator supporting traditional C for() loop
 *  and C++ 11 for() loop.
 *
 *  'list' and 'ring'
 *  at_eol is marked when we are at the end of the list,
 *  at_end is marked when we are at the end of the ring
 *  for explanation for 'list' and 'ring', please read 'iterator list' section
 *
 *  member 'current':
 *  _custom_it_t uses member 'current' rather than 'this' to pinpoint current list location. this way,
 *  a non-pointer object can be self-incremented to next location of the list.
 *
 *  iterator list:
 *  [ROOT ITERATOR], [1st OBJECT], ..., [n-th OBJECT], [EOL]
 *  |                |                  |              |
 *  |                |                  |              |
 *  |                |                  |              |
 *  |                |                  |              |
 *  |                |                  |              ----- symbolic iterator, access of data is denied. symbolic
 *  |                |                  |                    iterator contains 'begin', 'end' iterators. these
 *  |                |                  |                    iterators has invalid 'current' member.
 *  |                |                  |                    direct access is allowed
 *  |                |                  |
 *  |                |                  -------------------- end of the ring, current->at_end is marked.
 *  |                |                                       current->next is NULL
 *  |                |
 *  |                ---------------------------------------- normal list iterator.
 *  |                                                         only creatable by _custom_it_t::append()
 *  |
 *  --------------------------------------------------------- root iterator. created by user.
 *                                                            the minimum iterator list length is 1
 *                                                            (since pathname always contains '/')
 *                                                            current->at_start marked as true
 *                                                            as this is the root iterator
 *
 */

template < typename Type >
class _custom_it_t
{
private:
    /// gateway of data and node
    _custom_it_t * current = nullptr;

    /// only accessible by current. direct access is denied
    _custom_it_t * node_next = nullptr;     // next location. NULL if at the end of ring
    _custom_it_t * node_begin = nullptr;

    /// iterator is at the beginning of the array
    /// accessible by current if current is not nullptr
    bool at_start = false;

    /// iterator is at the end of the array
    /// accessible by current if current is not nullptr
    bool at_end = false;

    /// further than end, at the end of the list
    /// accessible by current if current is not nullptr
    bool at_eol = false;

    /// disable deconstruction in certain nodes (nodes allocated temporarily)
    /// only modified by append()
    bool _do_not_de_construct = true;

    /// iterator data, accessible by current
    Type data;

    /// length of current iterator, only make sense for node_begin
    uint64_t length = 0;

public:
    /// get data pointer
    Type & operator*();

    /// change position of current iterator
    const _custom_it_t operator++(int);
    const _custom_it_t operator++() { return this->operator++(0); }

    /// compare data
    bool operator == (_custom_it_t & new_data);
    bool operator != (_custom_it_t & new_data);

    /// append new data to iterator
    void append(Type &);

    /// get iterator ring size
    [[nodiscard]] htmpfs_size_t size() const { return current->node_begin->length; }

    /// de-constructor
    ~_custom_it_t();

    _custom_it_t & operator=(_custom_it_t&& new_data) noexcept
            { current = new_data.current; }

    /// create a link at either start or at the end of the link for compare purpose only
    /// @param position if position is true, then crate a beginning iterator, if position is false, then create
    ///                 an end iterator
    explicit _custom_it_t(bool position);

    /// copy constructor
    _custom_it_t(const _custom_it_t<Type>&);

    /// create an empty node at the start
    _custom_it_t();

    /// C++ 11 iterator API
    _custom_it_t begin(); // data accessible
    _custom_it_t end(); // !!! Please note that this is different from an end node !!!
                        // accessing this node is equivalent to accessing a nullptr
};

class path_t
{
public:
    typedef _custom_it_t < std::string > iterator;

private:
    iterator pathname;

public:
    explicit path_t (const std::string &);
    iterator begin()    { return pathname.begin(); }
    iterator end()      { return pathname.end(); }
    htmpfs_size_t size() { return pathname.size(); }
};

#include <custom_itr.h>

#endif //HTMPFS_PATH_T_H
