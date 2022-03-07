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

typedef uint64_t snapshot_ver_t;
typedef uint64_t buffer_id_t;
typedef uint64_t inode_id_t;
struct unique_buffer_pkg_id_t
{
    snapshot_ver_t version;
    buffer_id_t    buffer_id;
    inode_id_t     inode_id;
    uint64_t       pkg_link_count;
};

class inode_smi_t;
class directory_resolver_t;

struct buffer_result_t
{
    buffer_id_t id;
    buffer_t * data;
    uint64_t _is_snapshoted:1;
};

struct inode_result_t
{
    inode_id_t id;
    inode_t * inode;
};

/*
 * Index node
 *
 * index node, or inode, offers managed, block-lized buffers.
 * inode also offers managed snapshot volume creation/deletion
 *
 * */

class inode_t
{
private:
    htmpfs_size_t block_size;
    inode_id_t    inode_id;
    inode_smi_t * filesystem;

    bool is_dentry = false;

    /// only make sense for root inode
    std::map < snapshot_ver_t /* snapshot version */,
            std::vector < buffer_result_t > /* block map */
    > buffer_map;

public:
    /// public accessible dentry flag
    [[nodiscard]] bool __is_dentry() const { return is_dentry; }

    /// dentry status override
    /// !! FOR DEBUG PURPOSE ONLY !!
#ifdef CMAKE_BUILD_DEBUG
    void __override_dentry_flag(bool status) { is_dentry = status; }
#endif // CMAKE_BUILD_DEBUG

    /// file attributes. NOTE: this attribute is not maintained by any member of inode_t
    /// if you wish to use this attribute, you have to update the information manually
    struct stat fs_stat { };

    /// initialize block
    explicit inode_t(htmpfs_size_t _block_size,
                     inode_id_t    _inode_id,
                     inode_smi_t * _filesystem,
                     bool _is_dentry = false);

    /// write(buffer, length, offset, resize)
    /// write is only available for snapshot version 0
    /// @param buffer write buffer
    /// @param length write length
    /// @param offset write offset
    /// @param resize if append == true, append buffer if length+offset > buffer size
    ///               if append == false, ignore buffer beyond buffer size
    /// @param dentry_only set status of current operation, true if inode is dentry
    ///                    this operation can only override by directory_resolver_t
    /// @return length of buffer written
    htmpfs_size_t write(const char * buffer,
                        htmpfs_size_t length,
                        htmpfs_size_t offset,
                        bool resize = true,
                        directory_resolver_t::__dentry_only dentry_only =
                                directory_resolver_t::__dentry_only(false));

    /// read(buffer, length, offset)
    /// @param buffer read buffer
    /// @param length read length
    /// @param offset read offset
    /// @return length of buffer read
    htmpfs_size_t read(snapshot_ver_t version,
                       char * buffer,
                       htmpfs_size_t length,
                       htmpfs_size_t offset);

    /// output buffer as string
    std::string to_string(snapshot_ver_t version);

    /// current buffer bank size by version
    htmpfs_size_t current_data_size(snapshot_ver_t version);

    /// create a new snapshot volume
    /// @param volume_version volume version, provided by user
    void create_new_volume(snapshot_ver_t volume_version);

    /// delete a snapshot volume
    /// @param volume_version volume version, provided by user
    void delete_volume(snapshot_ver_t volume_version);

    friend class inode_smi_t;
};

/*
 * Index-NODE System Management Interface
 *
 * Index-NODE System Management Interface, or inode_smi, is used for inode management.
 * inode_smi supports create/delete inodes, create/delete snapshot volumes.
 *
 * CREATE/DELETE inode
 *      inode_smi is initialized with a filesystem root, which is marked as dentry inode
 *      when creating inode, user must provide a valid parent inode number and a valid child
 *      name. this creation process only write a dentry in parent inode and allocate a free inode in
 *      inode pool (or increase inode link count).
 *
 *      similar to creating inode, deleting an inode will also need a parent inode number and a valid child name
 *      inode will be removed from the inode pool or link count will be decreased by one. dentry will also
 *      be removed from parent inode.
 *
 *      inode_smi supports full path parse (i.e. pathname like "/etc", "/usr/bin", "/snap/bare/current/dev")
 *      you can obtain a valid inode number as long as the provided pathname is valid.
 *      you can directly access inode by using inode_id, no additional parse required.
 *
 *      NOTE that inode can be linked to multiple dentries, so inode will remain valid as long as
 *      link count > 0 (inode will be automatically removed when link_count == 0)
 *
 * */

class inode_smi_t
{
private:
    /// buffer pack for vector storage
    struct buffer_pack_t
    {
        uint64_t link_count{};
        buffer_t buffer;
    };

    /// inode pack for vector storage
    struct inode_pack_t
    {
        uint64_t link_count{};
        inode_t  inode;
    };

    /// const value, current bank block size
    const htmpfs_size_t block_size;

    /// root inode
    inode_t * filesystem_root;

    /// block pool, auto deconstruction enabled
    std::map < buffer_id_t, buffer_pack_t > buffer_pool;

    /// inode pool
    std::map < inode_id_t, inode_pack_t > inode_pool;

    /// snapshot version list
    std::map < snapshot_ver_t, std::vector < inode_result_t > > snapshot_version_list;

    /// get a free id
    template<class Typename>
    uint64_t get_free_id(Typename & pool);

    /// REQUEST FUNCTIONS: ONLY INVOKABLE BY inode_t
    /// request allocating buffer
    buffer_result_t request_buffer_allocation();

    /// increase link of specific buffer
    void link_buffer(buffer_id_t buffer_id);

    /// request deletion of buffer
    void unlink_buffer(buffer_id_t buffer_id);

    /// increase link of specific inode
    void link_inode(inode_id_t inode_id);

    /// decrease link of specific inode
    void unlink_inode(inode_id_t inode_id);

    /// get buffer by buffer id
    /// @param buffer_id buffer id
    /// @return pointer to buffer
    buffer_t * get_buffer_by_id(buffer_id_t buffer_id);

public:
    explicit inode_smi_t(htmpfs_size_t _block_size);

    /// get inode pointer by path
    inode_id_t get_inode_id_by_path(const std::string&, snapshot_ver_t);

    /// get inode pointer by id
    inode_t * get_inode_by_id(inode_id_t inode_id);

    /// make a child dentry under parent, only for version 0
    /// parent must be a directory
    /// @param parent_inode_id parent inode id
    /// @param name name for the creation of inode
    /// @return new inode id
    inode_id_t make_child_dentry_under_parent(inode_id_t parent_inode_id,
                                             const std::string & name,
                                             bool is_dir = false);

    /// remove a child dentry under parent, only for version 0
    void remove_child_dentry_under_parent(inode_id_t parent_inode_id,
                                          const std::string & name);

    /// remove an inode by path, for debug purpose only
    void remove_inode_by_path(const std::string & pathname);

    /// create a snapshot volume
    /// @return snapshot volume version
    snapshot_ver_t create_snapshot_volume();

    /// create a snapshot volume
    /// @param version snapshot version
    void delete_snapshot_volume(snapshot_ver_t version);

    /// export current filesystem layout as filesystem map
    /// @retuen filesystem layout
    std::vector < std::string > export_as_filesystem_map(snapshot_ver_t version);

    friend class inode_t;
};

template<class Typename>
uint64_t inode_smi_t::get_free_id(Typename & pool)
{
    if (pool.size() == pool.max_size()) {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_BUFFER_ID_DEPLETED);
    }

    uint64_t id = pool.size();
    while (pool.find(id) != pool.end()) {
        id++;
    }

    return id;
}

#endif //HTMPFS_HTMPFS_H
