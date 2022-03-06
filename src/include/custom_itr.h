#ifndef HTMPFS_CUSTOM_ITR_H
#define HTMPFS_CUSTOM_ITR_H

#include <path_t.h>
#include <htmpfs_error.h>
#include <debug.h>

template<typename Type>
const _custom_it_t<Type>  _custom_it_t<Type>::operator++(int)
{
    // if current iterator is at the end of the ring
    if (!current->node_next)
    {
        // update information (safer route)
        at_eol = true;
        this->current = nullptr;
    }
    else
    {
        // update gateway
        this->current = current->node_next;
    }

    return *this;
}

template<typename Type>
bool _custom_it_t<Type>::operator==(_custom_it_t &new_data)
{
    // both are symbolic, both 'current' is unavailable
    if (!current && !new_data.current)
    {
        if ((new_data.at_start and at_start)
            or (new_data.at_eol and at_eol)
            or (new_data.at_end and at_end))
        {
            return true;
        }

        return false;
    }

    // I'm a symbolic node, 'current' access is unavailable
    if (!current)
    {
        if ((new_data.current->at_start and at_start)
        or (new_data.current->at_eol and at_eol)
        or (new_data.current->at_end and at_end))
        {
            return true;
        }

        return false;
    }

    // new_data is symbolic, new_data.current is unavailable
    if (!new_data.current)
    {
        if ((current->at_start and new_data.at_start)
            or (current->at_eol and new_data.at_eol)
            or (current->at_end and new_data.at_end))
        {
            return true;
        }

        return false;
    }

    // both are beginning iterator (data doesn't necessarily need to match)
    if ((new_data.current->at_start and current->at_start))
    {
        return true;
    }

    // hard match
    return current == new_data.current;
}

template<typename Type>
bool _custom_it_t<Type>::operator!=(_custom_it_t &new_data)
{
    return !this->operator==(new_data);
}

template<typename Type>
_custom_it_t<Type>::_custom_it_t(bool position /* true if a beginning iterator is pending to be created
                                                * false if otherwise*/)
{
    // accessing doesn't go through the 'current' gateway because this is the beginning iterator
    // and 'current' isn't initialized
    if (position)
    {
        at_end      = false;
        at_start    = true;
        current     = this;
        node_next   = nullptr;
        node_begin  = this;
        length      = 1;
    }
    else
    {
        at_end      = true;
        at_start    = false;
        current     = nullptr;
        node_next   = nullptr;
        node_begin  = nullptr;
    }
}

template<typename Type>
void _custom_it_t<Type>::append(Type & _data)
{
    // fill out info
    auto new_data       = new _custom_it_t<Type>();
    new_data->current   = new_data;
    new_data->node_next = nullptr;
    new_data->at_end    = true;
    new_data->at_start  = false;
    new_data->node_begin = current->node_begin;
    new_data->data      = _data;
    new_data->_do_not_de_construct = true;

    current->node_next = new_data;
    current->at_end = false;

    node_begin->length += 1;
    node_begin->_do_not_de_construct = false;

    current = new_data;
}

template<typename Type>
_custom_it_t<Type>::~_custom_it_t()
{
    if (!_do_not_de_construct && current)
    {
        _do_not_de_construct = true;

        auto * head = current->node_begin->node_next;
        while (head)
        {
            auto * target = head;
            head = head->node_next;
            delete target;
        }
    }
}

template<typename Type>
_custom_it_t<Type>::_custom_it_t()
{
    at_end      = false;
    at_start    = true;
    current     = this;
    node_next   = nullptr;
    node_begin  = this;
    length      = 1;
}

template<typename Type>
_custom_it_t<Type> _custom_it_t<Type>::begin()
{
    _custom_it_t<Type> ret { };
    ret.current->current = current->node_begin;
    ret.node_begin = current->node_begin;
    return ret;
}

template<typename Type>
_custom_it_t<Type> _custom_it_t<Type>::end()
{
    _custom_it_t<Type> ret;
    memset(&ret, 0, sizeof(ret));
    ret.at_eol = true;
    ret._do_not_de_construct = true;
    ret.at_start = false;
    return ret;
}

template<typename Type>
Type &_custom_it_t<Type>::operator*()
{
    if (current && !at_eol)
    {
        return current->data;
    }
    else
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_ILLEGAL_ACCESS,
                                  "Illegal access detected in iterator");
    }
}

template<typename Type>
_custom_it_t<Type>::_custom_it_t(const _custom_it_t<Type> &new_data)
{
    current = new_data.current;
}

#undef OUTPUT_COMMAND

#endif //HTMPFS_CUSTOM_ITR_H
