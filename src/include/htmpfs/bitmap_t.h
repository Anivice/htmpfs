#ifndef HTMPFS_BITMAP_T_H
#define HTMPFS_BITMAP_T_H

/** @file
 *  this file defines functions for a bitmap
 */

#include <cstdint>

/// set specific bit of val64 as 1
inline void set_bit_of(uint64_t & val64, uint64_t bit_offset)
{
    uint64_t comp = (uint64_t)0x01 << bit_offset;
    val64 |= comp;
}

/// set specific bit of val64 as 0
inline void remove_bit_of(uint64_t & val64, uint64_t bit_offset)
{
    uint64_t comp = (uint64_t)0x01 << bit_offset;
    comp = ~comp;
    val64 &= comp;
}

/// get specific bit of val64
inline bool get_bit_of(uint64_t val64, uint64_t bit_offset)
{
    uint64_t comp = (uint64_t)0x01 << bit_offset;
    return comp & val64;
}

class bitmap_t
{

};


#endif //HTMPFS_BITMAP_T_H
