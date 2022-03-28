/** @file
 *
 * This file implements operations for bitmap support
 */

#include <htmpfs/bitmap_t.h>

bitmap_t::bitmap_t(inode_smi_t *_filesystem,
                   inode_id_t _starting_buffer_id,
                   htmpfs_size_t _bitmap_length)
{
    filesystem = _filesystem;
    bitmap_length = _bitmap_length;
    starting_buffer_id = _starting_buffer_id;
}

void bitmap_t::set_bit_in_map(htmpfs_size_t location, int bit)
{
    htmpfs_size_t byte_location = location / 8;
    htmpfs_size_t bit_offset = location % 8;
    htmpfs_size_t byte_loc_in_block = byte_location / filesystem->get_block_size() + starting_buffer_id;
    htmpfs_size_t byte_off_in_block = byte_location % filesystem->get_block_size();
    uint8_t byte;

    // buffer location by ID
    auto buffer = filesystem->get_buffer_by_id(byte_loc_in_block);
    // read byte
    buffer->read((char*)&byte, 1, byte_off_in_block);

    // set bit as 1
    if (bit)
    {
        set_bit_of(byte, bit_offset);
    }
    else // set bit as 0
    {
        remove_bit_of(byte, bit_offset);
    }

    // write into buffer pool
    buffer->write((const char*)&byte, 1, byte_off_in_block, false);
}

bool bitmap_t::get_bit_in_map(htmpfs_size_t location)
{
    htmpfs_size_t byte_location = location / 8;
    htmpfs_size_t bit_offset = location % 8;
    htmpfs_size_t byte_loc_in_block = byte_location / filesystem->get_block_size() + starting_buffer_id;
    htmpfs_size_t byte_off_in_block = byte_location % filesystem->get_block_size();
    uint8_t byte;

    // buffer location by ID
    auto buffer = filesystem->get_buffer_by_id(byte_loc_in_block);
    // read byte
    buffer->read((char*)&byte, 1, byte_off_in_block);

    return get_bit_of(byte, bit_offset);
}
