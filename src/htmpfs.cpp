#include <htmpfs.h>
#include <algorithm>
#include <htmpfs_error.h>
#include <directory_resolver.h>
#include <sstream>

#define VERIFY_DATA_OPS_LEN(operation, len) \
    if ((operation) != len)                 \
    {                                       \
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_BUFFER_SHORT_WRITE); \
    } __asm__("nop")

inode_t::inode_t(uint64_t _block_size, inode_id_t _inode_id, inode_smi_t * _filesystem, bool _is_dentry)
: block_size(_block_size), inode_id(_inode_id), filesystem(_filesystem), is_dentry(_is_dentry)
{
    // create root snapshot
    buffer_map.emplace(0, std::vector < buffer_result_t >());
}

htmpfs_size_t inode_t::write(const char *buffer,
                             htmpfs_size_t length,
                             htmpfs_size_t offset,
                             bool resize,
                             directory_resolver_t::__dentry_only dentry_only)
{
    /**                     SANITY CHECK                    **/
    // if is_dir == true, write is only accessible by directory_resolver::save_current()
    if (__is_dentry())
    {
        if (!dentry_only.is_dentry_only)
        {
            THROW_HTMPFS_ERROR_STDERR(HTMPFS_INVALID_WRITE_INVOKE);
        }
    }

    if (!length)
    {
        return 0;
    }

    /**                     SANITY CHECK END                    **/

    auto &snapshot_0_block_list = buffer_map.at(0);

    // if resizing buffer
    if (resize)
    {
        // check buffer bank availability
        htmpfs_size_t current_bank_size = snapshot_0_block_list.size() * block_size;
        htmpfs_size_t length_after_write = offset + length;

        // meet bank size shortage, i.e., grow bank at the end of the buffer list
        if (current_bank_size < length_after_write)
        {
            // first, calculate how much additional buffer is required
            htmpfs_size_t wanted_size = length_after_write - current_bank_size;
            htmpfs_size_t wanted_buffer_count = wanted_size / block_size + (wanted_size % block_size != 0);

            // append these additional buffer
            for (htmpfs_size_t i = 0; i < wanted_buffer_count; i++)
            {
                // emplace lost buffer
                auto result = filesystem->request_buffer_allocation();
//                auto result = buffer_result_t { .id = 0x00, .buffer = new buffer_t };
                snapshot_0_block_list.emplace_back(result);
            }

            // second, check how many buffer (since offset) need to be reallocated
            // because of snapshot
            // before we do that, let's see how much existing buffer needs to be modified
            htmpfs_size_t existing_buffer_pending_for_modification_start = offset / block_size;
            // end - modification-starting-buffer
            htmpfs_size_t existing_buffer_pending_for_modification_count =
                    snapshot_0_block_list.size() - existing_buffer_pending_for_modification_start;

            // check all buffers pending for modification
            // reallocate buffer when detecting snapshot-frozen buffers
            for (uint64_t i = 0;
                i < existing_buffer_pending_for_modification_count;
                i++)
            {
                // if frozen buffer detected
                if ((buffer_map.at(0)[i + existing_buffer_pending_for_modification_start])._is_snapshoted)
                {
                    // read data from old buffer
                    char * tmp = new char [block_size];
                    auto frozen_buffer =
                            &buffer_map.at(0)[i + existing_buffer_pending_for_modification_start];
                    uint64_t len = frozen_buffer->data->read(tmp, block_size, 0);

                    // allocate new buffer
                    auto new_buffer = filesystem->request_buffer_allocation();
                    new_buffer.data->write(tmp, len, 0);

                    // replace buffer
                    buffer_map.at(0)[i + existing_buffer_pending_for_modification_start] = new_buffer;

                    delete []tmp;
                }
            }

        }
        else // buffer bank is larger than wanted size
        {
            // first, see how much buffer is pending for deletion
            htmpfs_size_t current_bank_count = snapshot_0_block_list.size();
            htmpfs_size_t bank_count_after_write =
                    length_after_write / block_size + (length_after_write % block_size != 0);
            htmpfs_size_t lost_buffer_count = current_bank_count - bank_count_after_write;

            // check frozen buffer in lost buffer list
            for (htmpfs_size_t i = 0; i < lost_buffer_count; i++)
            {
                auto last_buffer = *(--(snapshot_0_block_list.end()));

                // if not a snapshot frozen buffer (now becomes a owner-less buffer)
                if (!last_buffer._is_snapshoted)
                {
                    // delete lost buffer
                    filesystem->unlink_buffer(last_buffer.id);
                }

                // remove buffer in the buffer list
                snapshot_0_block_list.pop_back();
            }

            // second, check the buffer pending for modification
            htmpfs_size_t existing_buffer_pending_for_modification_start = offset / block_size;
            // end - modification-starting-buffer
            htmpfs_size_t existing_buffer_pending_for_modification_end
                = (offset + length) / block_size + (((offset + length) % block_size) != 0);

            // check all buffers pending for modification
            // reallocate buffer when detecting snapshot-frozen buffers
            for (uint64_t i = existing_buffer_pending_for_modification_start;
                 i < existing_buffer_pending_for_modification_end;
                 i++)
            {
                // if frozen buffer detected
                if ((buffer_map.at(0)[i])._is_snapshoted)
                {
                    // read data from old buffer
                    char * tmp = new char [block_size];
                    auto frozen_buffer = &buffer_map.at(0)[i];
                    uint64_t len = frozen_buffer->data->read(tmp, block_size, 0);

                    // allocate new buffer
                    auto new_buffer = filesystem->request_buffer_allocation();
                    new_buffer.data->write(tmp, len, 0);

                    // replace buffer
                    buffer_map.at(0)[i] = new_buffer;

                    delete []tmp;
                }
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
        if (offset > current_data_size(0)) // write beyond buffer bank
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

        // check the buffer pending for modification
        htmpfs_size_t existing_buffer_pending_for_modification_start = offset / block_size;
        // end - modification-starting-buffer
        htmpfs_size_t existing_buffer_pending_for_modification_end
                = (offset + write_size) / block_size + (((offset + write_size) % block_size) != 0);

        // check all buffers pending for modification
        // reallocate buffer when detecting snapshot-frozen buffers
        for (uint64_t i = existing_buffer_pending_for_modification_start;
             i < existing_buffer_pending_for_modification_end;
             i++)
        {
            // if frozen buffer detected
            if ((buffer_map.at(0)[i])._is_snapshoted)
            {
                // read data from old buffer
                char * tmp = new char [block_size];
                auto frozen_buffer = &buffer_map.at(0)[i];
                uint64_t len = frozen_buffer->data->read(tmp, block_size, 0);

                // allocate new buffer
                auto new_buffer = filesystem->request_buffer_allocation();
                new_buffer.data->write(tmp, len, 0);

                // replace buffer
                buffer_map.at(0)[i] = new_buffer;

                delete []tmp;
            }
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

htmpfs_size_t inode_t::read(snapshot_ver_t version,
                            char *buffer,
                            htmpfs_size_t length,
                            htmpfs_size_t offset)
{
    if (!length)
    {
        return 0;
    }

    if (buffer_map.find(version) == buffer_map.end())
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_NO_SUCH_SNAPSHOT);
    }

    auto &snapshot_block_list = buffer_map.at(version);

    htmpfs_size_t read_size;
    if (offset > current_data_size(version)) // read beyond buffer bank
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
    if (buffer_map.empty() || !current_data_size(version))
    {
        return "";
    }

    char * buffer = new char [block_size * buffer_map.at(version).size() + 1] { };
    std::string ret;

    auto read_len =
            read(version, buffer, block_size * buffer_map.at(version).size() + 1, 0);

    if (read_len != current_data_size(version))
    {
        delete []buffer;
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_BUFFER_SHORT_READ);
    }

    for (uint64_t i = 0; i < read_len; i++)
    {
        ret += buffer[i];
    }

    delete []buffer;
    return ret;
}

htmpfs_size_t inode_t::current_data_size(snapshot_ver_t version)
{
    auto it = buffer_map.find(version);
    if (it == buffer_map.end())
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_NO_SUCH_SNAPSHOT);
    }

    htmpfs_size_t size = 0;
    auto & vec = it->second;
    for (const auto & i : vec)
    {
        size += i.data->size();
    }

    return size;
}

void inode_t::create_new_volume(snapshot_ver_t volume_version)
{
    if (buffer_map.find(volume_version) != buffer_map.end())
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_DOUBLE_SNAPSHOT);
    }

    std::vector < buffer_result_t > new_volume;

    for (auto & i : buffer_map.at(0))
    {
        // frozen this buffer
        i._is_snapshoted = 1;
        new_volume.emplace_back(i);

        // create a new link for buffer
        filesystem->link_buffer(i.id);
    }

    buffer_map.emplace(volume_version, new_volume);
}

void inode_t::delete_volume(snapshot_ver_t volume_version)
{
    if ((volume_version == 0)
    || (buffer_map.find(volume_version) == buffer_map.end()))
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_NO_SUCH_SNAPSHOT);
    }

    for (auto & i : buffer_map.at(volume_version))
    {
        // create a new link for buffer
        filesystem->unlink_buffer(i.id);
    }

    buffer_map.erase(volume_version);
}

buffer_result_t inode_smi_t::request_buffer_allocation()
{
    auto id = get_free_id(buffer_pool);

    buffer_pool.emplace(id, buffer_pack_t
            {
                    .link_count = 1,
                    .buffer = buffer_t(),
            }
    );

    return buffer_result_t {
        .id = id,
        .data = &buffer_pool.at(id).buffer,
        ._is_snapshoted = 0
    };
}

void inode_smi_t::unlink_buffer(buffer_id_t buffer_id)
{
    // attempt to delete a non-exist buffer
    auto it = buffer_pool.find(buffer_id);
    if (it == buffer_pool.end())
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_REQUESTED_BUFFER_NOT_FOUND);
    }

    if (it->second.link_count == 1) {
        buffer_pool.erase(it);
    } else {
        it->second.link_count -= 1;
    }
}

inode_id_t inode_smi_t::get_inode_id_by_path(const std::string & path, snapshot_ver_t version)
{
    path_t vec_path(path);
    inode_id_t current_inode = 0;

    for (const auto & i : vec_path)
    {
        // ignore filesystem root
        if (i.empty()) { continue; }

        // get next level of inode
        directory_resolver_t directoryResolver(&inode_pool.at(current_inode).inode, version);
        current_inode = directoryResolver.namei(i);
    }

    return current_inode;
}

inode_smi_t::inode_smi_t(htmpfs_size_t _block_size)
: block_size(_block_size)
{
    inode_pool.emplace
    (
            0,
            inode_pack_t
            {
                .link_count = 1,
                .inode = inode_t(
                        _block_size,
                        0,
                        this,
                        true)
            }
    );

    filesystem_root = &inode_pool.at(0).inode;
    snapshot_version_list.emplace(0,
                                  std::vector <inode_result_t >
                                          ({
                                              inode_result_t
                                              {
                                                  .id = 0,
                                                  .inode = filesystem_root
                                              }
                                          })
    );
}

inode_id_t inode_smi_t::make_child_dentry_under_parent(inode_id_t parent_inode_id,
                                                       const std::string & name,
                                                       bool is_dir)
{
    for (const auto & i : name)
    {
        if (i == '/' || i < 0x1F || i >= 0x7F)
        {
            THROW_HTMPFS_ERROR_STDERR(HTMPFS_INVALID_DENTRY_NAME);
        }
    }

    // make sure parent inode is valid
    auto it = inode_pool.find(parent_inode_id);
    if (it == inode_pool.end())
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_REQUESTED_INODE_NOT_FOUND);
    }

    // get parent inode pointer
    inode_t * parent_inode = &it->second.inode;
    // directory resolver
    directory_resolver_t directoryResolver(parent_inode, 0);

    // check name availability
    if (!directoryResolver.check_availability(name))
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_DOUBLE_MKPATHNAME);
    }

    // make a new inode
    auto new_inode_id = get_free_id(inode_pool);
    inode_pool.emplace(
            new_inode_id,
            inode_pack_t
            {
                .link_count = 1,
                .inode = inode_t(block_size, new_inode_id, this, is_dir)
            }
    );

    directoryResolver.add_path(name, new_inode_id);
    directoryResolver.save_current();

    inode_result_t inodeResult
            {
        .id = new_inode_id,
        .inode = &inode_pool.at(new_inode_id).inode
            };

    snapshot_version_list.at(0).emplace_back(inodeResult);

    return new_inode_id;
}

void inode_smi_t::link_buffer(buffer_id_t buffer_id)
{
    // attempt to link a non-exist buffer
    auto it = buffer_pool.find(buffer_id);
    if (it == buffer_pool.end())
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_REQUESTED_BUFFER_NOT_FOUND);
    }

    it->second.link_count += 1;
}

void inode_smi_t::link_inode(inode_id_t inode_id)
{
    // attempt to link a non-exist inode
    auto it = inode_pool.find(inode_id);
    if (it == inode_pool.end())
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_REQUESTED_INODE_NOT_FOUND);
    }

    it->second.link_count += 1;
}

void inode_smi_t::unlink_inode(inode_id_t inode_id)
{
    // attempt to unlink a non-exist inode
    auto it = inode_pool.find(inode_id);
    if (it == inode_pool.end())
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_REQUESTED_INODE_NOT_FOUND);
    }

    it->second.link_count -= 1;

    if (it->second.link_count == 0)
    {
        inode_pool.erase(inode_id);
    }
}

inode_t *inode_smi_t::get_inode_by_id(inode_id_t inode_id)
{
    auto it = inode_pool.find(inode_id);
    if (it == inode_pool.end())
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_REQUESTED_INODE_NOT_FOUND);
    }

    return &it->second.inode;
}

void inode_smi_t::remove_child_dentry_under_parent(inode_id_t parent_inode_id, const std::string &name)
{
    // make sure parent inode is valid
    auto it = inode_pool.find(parent_inode_id);
    if (it == inode_pool.end())
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_REQUESTED_INODE_NOT_FOUND);
    }

    // get parent inode pointer
    inode_t * parent_inode = &it->second.inode;
    // directory resolver
    directory_resolver_t directoryResolver(parent_inode, 0);

    auto target_id = directoryResolver.namei(name);
    directoryResolver.remove_path(name);
    directoryResolver.save_current();
    auto target_it = inode_pool.find(target_id);

    try
    {
        __disable_output = true;
        directory_resolver_t if_target_is_dir(&target_it->second.inode, 0);
        if (if_target_is_dir.target_count() != 0)
        {
            __disable_output = false;
            THROW_HTMPFS_ERROR_STDERR(HTMPFS_DIR_NOT_EMPTY);
        }
    }
    catch (HTMPFS_error_t & error)
    {
        // target is a directory, but something else failed
        if (error.my_errcode() != HTMPFS_NOT_A_DIRECTORY)
        {
            throw;
        }
    }

    __disable_output = false;

    // remove inode in version 0
    auto * vec = &snapshot_version_list.at(0);
    for (auto vec_it = vec->begin(); vec_it != vec->end(); vec_it++)
    {
        if (vec_it->id == target_id)
        {
            vec->erase(vec_it);
            break;
        }
    }

    // remove link
    target_it->second.link_count -= 1;

    // if no link is associated to this inode, remove it
    if (target_it->second.link_count == 0)
    {
        inode_pool.erase(target_it);
    }
}

// don't touch this
// i literally modified nothing and it doesn't work for some reason. don't change it
// or you are going to waste more time than you can think of
std::vector < std::string > inode_smi_t::export_as_filesystem_map(snapshot_ver_t version)
{
    std::vector < std::string > filesystem_map;
    /* traverse filesystem
     * first, a function that shows all targets within the current directory
     * */

    // get inode pathname package by parent inode
    // basically list all inode entries in parent inode
    auto get_sub_inode_list_by_parent =
            [&](inode_t * parent_inode)->std::vector < directory_resolver_t::path_pack_t >
    {
        directory_resolver_t directoryResolver(parent_inode, version);
        return directoryResolver.to_vector();
    };

    auto make_path_for_current_dentry =
            [&](const std::string& pathname_prefix)->std::vector < std::string >
    {
        std::vector < std::string > ret;
        std::vector < directory_resolver_t::path_pack_t > current_sub_inode_list;

        // get parent inode
        auto * parent_inode =
                get_inode_by_id(
                        get_inode_id_by_path((pathname_prefix),
                                                    version)
                );

        // check if parent inode is a dentry inode
        if (!parent_inode->__is_dentry())
        {
            return {};
        }

        // get sub inode list under dentry inode
        current_sub_inode_list = get_sub_inode_list_by_parent(parent_inode);

        // make pathname
        for (const auto & i : current_sub_inode_list)
        {
//            sys_map.emplace_back(pathname_prefix + "/" + i.pathname);
//            std::cout << pathname_prefix + i.pathname << std::endl;
            filesystem_map.emplace_back(pathname_prefix + i.pathname);
            ret.emplace_back(pathname_prefix + i.pathname);
        }

        return ret;
    };

    // recursively check filesystem tree
    std::function < void (const std::vector < std::string > &) >
            recursion_check = [&](const std::vector < std::string /* full pathname */ >& dentry_list)->void
    {
        for (const auto &i: dentry_list)
        {
            if (i == "/") {
                recursion_check(make_path_for_current_dentry("/"));
            } else {
                recursion_check(make_path_for_current_dentry(i + "/"));
            }
        }
    };

    recursion_check(std::vector <std::string>({"/"}));

    return filesystem_map;
}

buffer_t *inode_smi_t::get_buffer_by_id(buffer_id_t buffer_id)
{
    auto it = buffer_pool.find(buffer_id);
    if (it == buffer_pool.end())
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_REQUESTED_BUFFER_NOT_FOUND);
    }

    return &it->second.buffer;
}

snapshot_ver_t inode_smi_t::create_snapshot_volume()
{
    auto snapshot_ver = get_free_id(snapshot_version_list);
    auto root_vec = snapshot_version_list.at(0);
    for (auto i : root_vec)
    {
        link_inode(i.id);
        i.inode->create_new_volume(snapshot_ver);
    }

    snapshot_version_list.emplace(snapshot_ver, root_vec);

    return snapshot_ver;
}

void inode_smi_t::delete_snapshot_volume(snapshot_ver_t version)
{
    if (snapshot_version_list.find(version) == snapshot_version_list.end())
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_NO_SUCH_SNAPSHOT);
    }

    auto target_vec = snapshot_version_list.at(version);
    for (auto i : target_vec)
    {
        i.inode->delete_volume(version);
        unlink_inode(i.id);
    }

    snapshot_version_list.erase(version);
}

void inode_smi_t::remove_inode_by_path(const std::string &pathname)
{
    if (pathname == "/")
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_CANNOT_REMOVE_ROOT);
    }

    // parse pathname
    path_t path(pathname);
    inode_t * ops_inode = filesystem_root;
    std::string target_name = *path.last();

    auto it = ++path.begin();
    for (uint64_t i = 0; i< path.size() - 1 /* filesystem root */ - 1 /* last target */; i++)
    {
        directory_resolver_t directoryResolver(ops_inode, 0);
        auto inode_id = directoryResolver.namei(*it);
        ops_inode = get_inode_by_id(inode_id);
        it++;
    }

    // remove child
    remove_child_dentry_under_parent(ops_inode->inode_id, target_name);
}

