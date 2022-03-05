#ifndef HTMPFS_PATH_T_H
#define HTMPFS_PATH_T_H

#include <string>
#include <vector>
#include <buffer_t.h>

class path_t;

template < typename Type >
class _custom_it_t
{
private:
    /// gateway of data and node
    _custom_it_t * current = nullptr;

    /// only accessible by current. direct access is denied
    _custom_it_t * node_next = nullptr;
    _custom_it_t * node_begin = nullptr;

    /// iterator is at the beginning of the array
    /// accessible by current
    bool at_start = false;

    /// iterator is at the end of the array
    /// accessible by current
    bool at_end = false;

    ///
    bool at_eol = false;

    /// disable deconstruction in certain nodes (nodes allocated temporarily)
    bool _do_not_de_construct = true;

    /// iterator data
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
