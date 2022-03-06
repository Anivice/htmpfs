#include <htmpfs.h>
#include <algorithm>
#include <htmpfs_error.h>

#define VERIFY_DATA_OPS_LEN(operation, len) \
    if ((operation) != len)                 \
    {                                       \
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_BUFFER_SHORT_WRITE); \
    } __asm__("nop")

inode_t::inode_t(uint64_t _block_size, uint32_t _inode_id, filesystem_t * _filesystem)
: block_size(_block_size), inode_id(_inode_id), filesystem(_filesystem)
{
    // create root snapshot
    block_map.emplace(0, std::vector < buffer_result_t >());
}

htmpfs_size_t inode_t::write(const char *buffer, htmpfs_size_t length, htmpfs_size_t offset, bool resize)
{
    auto &snapshot_0_block_list = block_map.at(0);

    // if resizing data
    if (resize)
    {
        // check data bank availability
        htmpfs_size_t current_bank_size = snapshot_0_block_list.size() * block_size;
        htmpfs_size_t length_after_write = offset + length;

        // meet bank size shortage
        if (current_bank_size < length_after_write)
        {
            htmpfs_size_t wanted_size = length_after_write - current_bank_size;
            htmpfs_size_t wanted_buffer_count = wanted_size / block_size + (wanted_size % block_size != 0);
            for (htmpfs_size_t i = 0; i < wanted_buffer_count; i++)
            {
                // emplace lost buffer
                auto result = filesystem->request_buffer_allocation();
//                auto result = buffer_result_t { .id = 0x00, .data = new buffer_t };
                snapshot_0_block_list.emplace_back(result);
            }
        }
        else // data bank is larger than wanted size
        {
            htmpfs_size_t current_bank_count = snapshot_0_block_list.size();
            htmpfs_size_t bank_count_after_write =
                    length_after_write / block_size + (length_after_write % block_size != 0);
            htmpfs_size_t lost_buffer_count = current_bank_count - bank_count_after_write;
            for (htmpfs_size_t i = 0; i < lost_buffer_count; i++) {
                // emplace lost buffer
                filesystem->request_buffer_deletion((--snapshot_0_block_list.end())->id);
                snapshot_0_block_list.pop_back();
            }
        }

        htmpfs_size_t offset_for_starting_buffer = offset % block_size;
        buffer_id_t starting_buffer = 0;
        htmpfs_size_t write_length_in_starting_buffer = 0;
        htmpfs_size_t write_length_in_last_buffer = 0;
        htmpfs_size_t full_block_operation_count = 0;

        // if offset == 0; then starting_buffer == 0;
        // if offset != 0, but offset < block_size, then starting_buffer = 0
        if ((!offset) or (offset < block_size)) { }
        // offset == block_size starting_buffer == 1
        else if (offset == block_size) { starting_buffer = 1; }
        // if offset > block_size, then starting_buffer = offset / block_size
        else if (offset > block_size)
        {
            starting_buffer = offset / block_size;
        }

        /*
         *     A       B       C       D       E       F
         * ++++++++.+++|===.=======.=======.=======.==|
         * .-------.---|---.-------.-------.-------.--|----.-------.-------.
         * |       |       |       |       |       |       |       |       |
         * .-------.-------.-------.-------.-------.-------.-------.-------.
         */

        // write_length_in_starting_buffer
        if ((block_size - offset_for_starting_buffer) /* remaining space in starting buffer */
            >= length) // all write operation is in starting buffer
        {
            write_length_in_starting_buffer = length;
        }
        else // write length is beyond starting buffer
        {
            write_length_in_starting_buffer = block_size - offset_for_starting_buffer;
        }

        // full_block_operation_count
        htmpfs_size_t remaining_write_length = length - write_length_in_starting_buffer;
        full_block_operation_count = remaining_write_length / block_size;

        // write_length_in_last_buffer
        write_length_in_last_buffer = remaining_write_length % block_size;

        // =================================================================== //

        // write starting block
        VERIFY_DATA_OPS_LEN(snapshot_0_block_list.at(starting_buffer).data->write(
                            buffer,
                            write_length_in_starting_buffer,
                            offset_for_starting_buffer,
                            true
        ), write_length_in_starting_buffer);

        // full block operation
        for (buffer_id_t i = 1; i <= full_block_operation_count; i++)
        {
            VERIFY_DATA_OPS_LEN(snapshot_0_block_list.at(starting_buffer + i).data->write(
                    buffer + write_length_in_starting_buffer + (i - 1) * block_size,
                    block_size,
                    0, true
            ), block_size);
        }

        // last block operation
        if (write_length_in_last_buffer)
        {
            VERIFY_DATA_OPS_LEN
            (
                    snapshot_0_block_list.at(starting_buffer + full_block_operation_count + 1).data->write(
                            buffer + write_length_in_starting_buffer + full_block_operation_count * block_size,
                            write_length_in_last_buffer,
                            0, true
                    ), write_length_in_last_buffer);
        }

        return length;
    }
    else // resize disabled
    {
        htmpfs_size_t write_size;
        if (offset > current_data_size(0)) // write beyond data bank
        {
            return 0;
        }
        else if (offset + length > current_data_size(0)) // bank size shortage
        {
            write_size = current_data_size(0) - offset;
        }
        else // write length OK
        {
            write_size = length;
        }

        // =================================================================== //

        htmpfs_size_t offset_for_starting_buffer = offset % block_size;
        buffer_id_t starting_buffer = 0;
        htmpfs_size_t write_length_in_starting_buffer = 0;
        htmpfs_size_t write_length_in_last_buffer = 0;
        htmpfs_size_t full_block_operation_count = 0;

        // if offset == 0; then starting_buffer == 0;
        // if offset != 0, but offset < block_size, then starting_buffer = 0
        if ((!offset) or (offset < block_size)) { }
        // offset == block_size starting_buffer == 1
        else if (offset == block_size) { starting_buffer = 1; }
        // if offset > block_size, then starting_buffer = offset / block_size
        else if (offset > block_size)
        {
            starting_buffer = offset / block_size;
        }

        // write_length_in_starting_buffer
        if ((block_size - offset_for_starting_buffer) /* remaining space in starting buffer */
            >= write_size) // all write operation is in starting buffer
        {
            write_length_in_starting_buffer = write_size;
        }
        else // write length is beyond starting buffer
        {
            write_length_in_starting_buffer = block_size - offset_for_starting_buffer;
        }

        // full_block_operation_count
        htmpfs_size_t remaining_write_length = write_size - write_length_in_starting_buffer;
        full_block_operation_count = remaining_write_length / block_size;

        // write_length_in_last_buffer
        write_length_in_last_buffer = remaining_write_length % block_size;

        // =================================================================== //

        // write starting block
        VERIFY_DATA_OPS_LEN(snapshot_0_block_list.at(starting_buffer).data->write(
                buffer,
                write_length_in_starting_buffer,
                offset_for_starting_buffer
        ), write_length_in_starting_buffer);

        // full block operation
        for (buffer_id_t i = 1; i <= full_block_operation_count; i++)
        {
            VERIFY_DATA_OPS_LEN(snapshot_0_block_list.at(starting_buffer + i).data->write(
                    buffer + write_length_in_starting_buffer + (i - 1) * block_size,
                    block_size,
                    0
            ), block_size);
        }

        // last block operation
        if (write_length_in_last_buffer)
        {
            VERIFY_DATA_OPS_LEN
            (
                    snapshot_0_block_list.at(starting_buffer + full_block_operation_count + 1).data->write(
                            buffer + write_length_in_starting_buffer + full_block_operation_count * block_size,
                            write_length_in_last_buffer,
                            0
                    ), write_length_in_last_buffer);
        }

        return write_size;
    }
}

htmpfs_size_t inode_t::read(snapshot_ver_t version, char *buffer, htmpfs_size_t length, htmpfs_size_t offset)
{
    auto &snapshot_block_list = block_map.at(version);

    htmpfs_size_t read_size;
    if (offset > current_data_size(version)) // read beyond data bank
    {
        return 0;
    }
    else if (offset + length > current_data_size(version)) // bank size shortage
    {
        read_size = current_data_size(version) - offset;
    }
    else // read length OK
    {
        read_size = length;
    }

    // =================================================================== //

    htmpfs_size_t offset_for_starting_buffer = offset % block_size;
    buffer_id_t starting_buffer = 0;
    htmpfs_size_t read_length_in_starting_buffer = 0;
    htmpfs_size_t read_length_in_last_buffer = 0;
    htmpfs_size_t full_block_operation_count = 0;

    // if offset == 0; then starting_buffer == 0;
    // if offset != 0, but offset < block_size, then starting_buffer = 0
    if ((!offset) or (offset < block_size)) { }
    // offset == block_size starting_buffer == 1
    else if (offset == block_size) { starting_buffer = 1; }
    // if offset > block_size, then starting_buffer = offset / block_size
    else if (offset > block_size)
    {
        starting_buffer = offset / block_size;
    }

    // read_length_in_starting_buffer
    if ((block_size - offset_for_starting_buffer) /* remaining space in starting buffer */
        >= read_size) // all read operation is in starting buffer
    {
        read_length_in_starting_buffer = read_size;
    }
    else // read length is beyond starting buffer
    {
        read_length_in_starting_buffer = block_size - offset_for_starting_buffer;
    }

    // full_block_operation_count
    htmpfs_size_t remaining_write_length = read_size - read_length_in_starting_buffer;
    full_block_operation_count = remaining_write_length / block_size;

    // read_length_in_last_buffer
    read_length_in_last_buffer = remaining_write_length % block_size;

    // =================================================================== //

    // write starting block
    VERIFY_DATA_OPS_LEN(snapshot_block_list.at(starting_buffer).data->read(
            buffer,
            read_length_in_starting_buffer,
            offset_for_starting_buffer
    ), read_length_in_starting_buffer);

    // full block operation
    for (buffer_id_t i = 1; i <= full_block_operation_count; i++)
    {
        VERIFY_DATA_OPS_LEN(snapshot_block_list.at(starting_buffer + i).data->read(
                buffer + read_length_in_starting_buffer + (i - 1) * block_size,
                block_size,
                0
        ), block_size);
    }

    // last block operation
    if (read_length_in_last_buffer)
    {
        VERIFY_DATA_OPS_LEN
        (
                snapshot_block_list.at(starting_buffer + full_block_operation_count + 1).data->read(
                        buffer + read_length_in_starting_buffer + full_block_operation_count * block_size,
                        read_length_in_last_buffer,
                        0
                ), read_length_in_last_buffer);
    }

    return read_size;
}

std::string inode_t::to_string(snapshot_ver_t version)
{
    if (block_map.empty() || block_map.find(version) == block_map.end() || !current_data_size(version))
    {
        return "";
    }

    char * buffer = new char [ block_size * block_map.at(version).size() + 1] { };
    std::string ret;

    auto read_len =
            read(version, buffer, block_size * block_map.at(version).size() + 1, 0);

    for (uint64_t i = 0; i < read_len; i++)
    {
        ret += buffer[i];
    }

    delete []buffer;
    return ret;
}

htmpfs_size_t inode_t::current_data_size(snapshot_ver_t version)
{
    auto it = block_map.find(version);
    if (it == block_map.end())
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_REQUESTED_VERSION_NOT_FOUND);
    }

    htmpfs_size_t size = 0;
    auto & vec = it->second;
    for (const auto & i : vec)
    {
        size += i.data->size();
    }

    return size;
}

buffer_result_t filesystem_t::request_buffer_allocation()
{
    auto id = get_free_id(buffer_pool);

    buffer_pool.emplace(id, buffer_pack_t
            {
                    .link_count = 1,
                    .data = buffer_t()
            }
    );

    return buffer_result_t {
        .id = id,
        .data = &buffer_pool.at(id).data
    };
}

void filesystem_t::request_buffer_deletion(buffer_id_t buffer_id)
{
    auto it = buffer_pool.find(buffer_id);
    if (it == buffer_pool.end())
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_REQUESTED_BUFFER_NOT_FOUND);
    }

    buffer_pool.erase(it);
}

inode_t *filesystem_t::get_inode_by_path(const std::string & path, snapshot_ver_t)
{
    path_t vec_path(path);
    inode_id_t current_inode = 0;

    for (const auto & i : vec_path)
    {
        // ignore filesystem root
        if (i.empty()) { continue; }

        // get next level of inode

    }

    return nullptr;
}

filesystem_t::filesystem_t(htmpfs_size_t _block_size)
: block_size(_block_size)
{
    inode_pool.emplace(0, inode_t(_block_size, 0, this));
    filesystem_root = &inode_pool.at(0);

}

inode_id_t filesystem_t::make_child_dentry_under_parent(inode_id_t inode_id,
                                                       const std::string & name)
{

}

void filesystem_t::remove_child_dentry_under_parent(inode_id_t inode_id)
{

}
