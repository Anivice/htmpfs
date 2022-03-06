#ifndef HTMPFS_HTMPFS_H
#define HTMPFS_HTMPFS_H

/** @file
 *  general htmpfs specified classes
 */

#include <debug.h>
#include <htmpfs_error.h>
#include <cstdint>
#include <buffer_t.h>
#include <uni_utils.h>
#include <map>
#include <string>
#include <path_t.h>
#include <directory_resolver.h>

#define LEFT_SHIFT64(val, bit_count)  ((uint64_t)((uint64_t)(val) << (uint64_t)bit_count))
#define RIGHT_SHIFT64(val, bit_count) ((uint64_t)((uint64_t)(val) >> (uint64_t)bit_count))

#define FILESYSTEM_ROOT_INODE_NUMBER    0x00
#define FILESYSTEM_CUR_MODIFIABLE_VER   0x00

typedef uint32_t snapshot_ver_t;
typedef uint64_t buffer_id_t;
typedef uint64_t inode_id_t;

class filesystem_t;

struct buffer_result_t
{
    buffer_id_t id;
    buffer_t * data;
};

class inode_t
{
private:
    htmpfs_size_t block_size;
    uint32_t inode_id;
    filesystem_t * filesystem;

    /// only make sense for root inode
    std::map < snapshot_ver_t /* snapshot version */,
            std::vector < buffer_result_t > /* block map */
    > block_map;

public:
    /// file attributes. NOTE: this attribute is not maintained by any member of inode_t
    /// if you wish to use this attribute, you have to update the information manually
    struct stat fs_stat { };

    /// initialize block
    explicit inode_t(htmpfs_size_t _block_size, uint32_t _inode_id, filesystem_t * _filesystem);

    /// write(buffer, length, offset, resize)
    /// write is only available for snapshot version 0
    /// @param buffer write buffer
    /// @param length write length
    /// @param offset write offset
    /// @param resize if append == true, append data if length+offset > buffer size
    ///               if append == false, ignore data beyond buffer size
    /// @return length of data written
    htmpfs_size_t write(const char * buffer, htmpfs_size_t length, htmpfs_size_t offset, bool resize = true);

    /// read(buffer, length, offset)
    /// @param buffer read buffer
    /// @param length read length
    /// @param offset read offset
    /// @return length of data read
    htmpfs_size_t read(snapshot_ver_t version, char * buffer, htmpfs_size_t length, htmpfs_size_t offset);

    /// output buffer as string
    std::string to_string(snapshot_ver_t version);

    /// current data bank size by version
    htmpfs_size_t current_data_size(snapshot_ver_t version);
};

class filesystem_t
{
private:
    struct buffer_pack_t
    {
        uint64_t link_count{};
        buffer_t data;
    };

    htmpfs_size_t block_size;

    /// root inode
    inode_t * filesystem_root;

    /// block pool, auto deconstruction enabled
    std::map < buffer_id_t, buffer_pack_t > buffer_pool;

    /// inode pool
    std::map < inode_id_t, inode_t > inode_pool;

    /// get a free buffer id
    template<class Typename>
    uint64_t get_free_id(Typename & pool);

    /// REQUEST FUNCTIONS: ONLY INVOKABLE BY inode_t
    /// request allocating buffer
    buffer_result_t request_buffer_allocation();

    /// request deletion of buffer
    void request_buffer_deletion(buffer_id_t);

public:
    explicit filesystem_t(htmpfs_size_t _block_size);

    /// get inode pointer by path
    inode_id_t get_inode_by_path(const std::string&, snapshot_ver_t);

    /// make a child dentry under parent, only for version 0
    /// parent must be a directory
    /// @param parent_inode_id parent inode id
    /// @param name name for the creation of inode
    /// @return new inode id
    inode_id_t make_child_dentry_under_parent(inode_id_t parent_inode_id,
                                             const std::string & name);

    /// remove a child dentry under parent, only for version 0
    void remove_child_dentry_under_parent(inode_id_t parent_inode_id);

    friend inode_t;
};

template<class Typename>
uint64_t filesystem_t::get_free_id(Typename & pool)
{
    if (pool.size() == pool.max_size())
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_BUFFER_ID_DEPLETED);
    }

    uint64_t id = pool.size();
    while (pool.find(id) != pool.end())
    {
        id++;
    }

    return id;
}

#endif //HTMPFS_HTMPFS_H
