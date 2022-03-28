#ifndef HTMPFS_BITMAP_T_H
#define HTMPFS_BITMAP_T_H

/** @file
 *  this file defines functions for a bitmap
 */

#include <cstdint>
#include <htmpfs/htmpfs.h>

/// set specific bit of val8 as 1
inline void set_bit_of(uint8_t & val8, uint8_t bit_offset)
{
    uint8_t comp = (uint8_t)0x01 << bit_offset;
    val8 |= comp;
}

/// set specific bit of val64 as 0
inline void remove_bit_of(uint8_t & val8, uint8_t bit_offset)
{
    uint8_t comp = (uint8_t)0x01 << bit_offset;
    comp = ~comp;
    val8 &= comp;
}

/// get specific bit of val64
inline bool get_bit_of(uint8_t val8, uint8_t bit_offset)
{
    uint8_t comp = (uint8_t)0x01 << bit_offset;
    return comp & val8;
}

class bitmap_t
{
    inode_smi_t * filesystem;
    htmpfs_size_t bitmap_length;
    buffer_id_t starting_buffer_id;

public:
    bitmap_t (inode_smi_t * _filesystem,
              buffer_id_t _starting_buffer_id,
              htmpfs_size_t _bitmap_length);

    /// set specific bit in bitmap
    void set_bit_in_map(htmpfs_size_t location, int bit);

    /// get specific bit in bitmap
    bool get_bit_in_map(htmpfs_size_t location);
};


#endif //HTMPFS_BITMAP_T_H
