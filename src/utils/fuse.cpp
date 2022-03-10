/** @file
 *
 * This file implements the APIs for FUSE
 */

#define FUSE_USE_VERSION 31
#include <fuse_ops.h>
#include <unistd.h>
#include <sys/param.h>
#include <uni_utils.h>

#define SNAPSHOT_ENTRY ".snapshot"
SmartPtr < inode_smi_t > filesystem_inode_smi;

#define CATCH_TAIL                                                                              \
catch (HTMPFS_error_t & error)                                                                  \
{                                                                                               \
    std::cerr << error.what() << " (errno=" << strerror(error.my_errno()) << ")" << std::endl;  \
    return -error.my_errno();                                                                   \
}                                                                                               \
catch (std::exception & error)                                                                  \
{                                                                                               \
    std::cerr << error.what() << " (errno=" << strerror(errno) << ")" << std::endl;             \
    return -errno;                                                                              \
} __asm__("nop")

#define CHECK_RDONLY_FS(path)                                           \
{                                                                       \
    std::string parsed_path;                                            \
    snapshot_ver_t version = if_snapshot(path, parsed_path);            \
    if (version != FILESYSTEM_CUR_MODIFIABLE_VER)                       \
    {                                                                   \
        return -EROFS;                                                  \
    }                                                                   \
} __asm__("nop")

int do_getattr (const char *path, struct stat *stbuf)
{
    try
    {
        if (!strcmp("/" SNAPSHOT_ENTRY , path)) // non-existing directory
        {
            stbuf->st_ctim  = get_current_time();
            stbuf->st_atim  = get_current_time();
            stbuf->st_mtim  = get_current_time();
            stbuf->st_ino   = 0;
            stbuf->st_mode  = 0755 | S_IFDIR;
            stbuf->st_gid   = getgid();
            stbuf->st_uid   = getuid();
            stbuf->st_nlink = 1;
            stbuf->st_size  = 0;
            return 0;
        }

        std::string parsed_path;
        snapshot_ver_t version = if_snapshot(path, parsed_path);
        auto inode_id = filesystem_inode_smi->get_inode_id_by_path(path);
        auto inode = filesystem_inode_smi->get_inode_by_id(inode_id);

        *stbuf = inode->fs_stat;
        stbuf->st_size = (off_t)inode->current_data_size(version);
        stbuf->st_nlink = filesystem_inode_smi->count_link_for_inode(inode_id);
        stbuf->st_ino = inode_id;
        return 0;
    }
    CATCH_TAIL;
}

int do_readdir (const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t,
                struct fuse_file_info *)
{
    try
    {
        filler(buffer, ".", nullptr, 0);  // Current Directory
        filler(buffer, "..", nullptr, 0); // Parent Directory

        if (!strcmp("/", path))
        {
            filler(buffer, SNAPSHOT_ENTRY, nullptr, 0); // .snapshot volume
        }

        if (!strcmp("/" SNAPSHOT_ENTRY, path)) // non-existing directory
        {
            for (const auto& i : filesystem_inode_smi->_snapshot_version_list)
            {
                filler(buffer, i.first.c_str(), nullptr, 0);
            }

            return 0;
        }

        std::string parsed_path;
        snapshot_ver_t version = if_snapshot(path, parsed_path);
        auto inode_id = filesystem_inode_smi->get_inode_id_by_path(path);
        auto inode = filesystem_inode_smi->get_inode_by_id(inode_id);
        directory_resolver_t directoryResolver(inode, version);

        auto dentry_list = directoryResolver.to_vector();
        for (const auto &i: dentry_list)
        {
            filler(buffer, i.pathname.c_str(), nullptr, 0);
        }

        return 0;
    }
    CATCH_TAIL;
}

int do_mkdir (const char * path, mode_t mode)
{
    try
    {
        path_t vpath(path);
        std::string target_name = vpath.pop_end();

        if (target_name.empty())
        {
            THROW_HTMPFS_ERROR_STDERR(HTMPFS_INVALID_DENTRY_NAME);
        }

        // now, determine if creating snapshot volume or normal directory
        if (vpath.size() == 2 /* {"", ".snapshot"} */ &&
            vpath.last()->length() == strlen(SNAPSHOT_ENTRY) &&
            !memcmp(vpath.last()->c_str(),
                    SNAPSHOT_ENTRY,vpath.last()->length())
                )
        {
            // it's a snapshot creation
            filesystem_inode_smi->create_snapshot_volume(target_name);
        }
        else
        {
            CHECK_RDONLY_FS(path);

            auto parent_id = filesystem_inode_smi->get_inode_id_by_path(vpath.to_string());
            auto child_id = filesystem_inode_smi->
                    make_child_dentry_under_parent(parent_id, target_name, true);
            auto child_inode = filesystem_inode_smi->get_inode_by_id(child_id);
            child_inode->fs_stat.st_size = 0;
            child_inode->fs_stat.st_ctim = get_current_time();
            child_inode->fs_stat.st_atim = get_current_time();
            child_inode->fs_stat.st_mtim = get_current_time();
            child_inode->fs_stat.st_ino = FILESYSTEM_ROOT_INODE_NUMBER;
            child_inode->fs_stat.st_mode = mode | S_IFDIR;
            child_inode->fs_stat.st_nlink = 1;
            child_inode->fs_stat.st_size = 0;
        }

        return 0;
    }
    CATCH_TAIL;
}

int do_chmod (const char * path, mode_t mode)
{
    try
    {
        CHECK_RDONLY_FS(path);

        auto inode_id = filesystem_inode_smi->get_inode_id_by_path(path);
        auto inode = filesystem_inode_smi->get_inode_by_id(inode_id);
        inode->fs_stat.st_mode = mode;
        inode->fs_stat.st_ctim = get_current_time();

        return 0;
    }
    CATCH_TAIL;
}

int do_chown (const char * path, uid_t uid, gid_t gid)
{
    try
    {
        CHECK_RDONLY_FS(path);

        auto inode_id = filesystem_inode_smi->get_inode_id_by_path(path);
        auto inode = filesystem_inode_smi->get_inode_by_id(inode_id);
        inode->fs_stat.st_uid = uid;
        inode->fs_stat.st_gid = gid;
        inode->fs_stat.st_ctim = get_current_time();

        return 0;
    }
    CATCH_TAIL;
}

int do_create (const char * path, mode_t mode, struct fuse_file_info *)
{
    try
    {
        CHECK_RDONLY_FS(path);

        path_t vpath(path);
        auto target_name = vpath.pop_end();

        if (vpath.size() == 0)
        {
            THROW_HTMPFS_ERROR_STDERR(HTMPFS_INVALID_DENTRY_NAME);
        }

        auto inode_id = filesystem_inode_smi->get_inode_id_by_path(vpath.to_string());
        auto new_target_id = filesystem_inode_smi->make_child_dentry_under_parent(inode_id,
                                                             target_name);
        auto new_inode = filesystem_inode_smi->get_inode_by_id(new_target_id);

        // fill up info
        auto cur_time = get_current_time();
        new_inode->fs_stat.st_mode = mode;
        new_inode->fs_stat.st_nlink = 1;
        new_inode->fs_stat.st_atim = cur_time;
        new_inode->fs_stat.st_ctim = cur_time;
        new_inode->fs_stat.st_mtim = cur_time;
        new_inode->fs_stat.st_gid = getgid();
        new_inode->fs_stat.st_uid = getuid();

        return 0;
    }
    CATCH_TAIL;
}

int do_flush (const char * path, struct fuse_file_info *)
{
    return 0;
}

int do_release (const char * path, struct fuse_file_info *)
{
    return 0;
}

int do_access (const char * path, int mode)
{
    try {
        if (!strcmp("/" SNAPSHOT_ENTRY, path)) // non-existing directory
        {
            return 0;
        }

        auto inode = filesystem_inode_smi->get_inode_by_id(
                filesystem_inode_smi->get_inode_id_by_path(path));

        if (mode == F_OK)
        {
            return 0;
        }

        mode <<= 6;
        mode &= 0x01C0;

        return -!(mode & inode->fs_stat.st_mode);
    }
    CATCH_TAIL;
}

int do_open (const char * path, struct fuse_file_info * info)
{
    try
    {
        if (!strcmp("/" SNAPSHOT_ENTRY , path)) // non-existing directory
        {
            return 0;
        }

        filesystem_inode_smi->get_inode_id_by_path(path);
        return 0;
    }
    CATCH_TAIL;
}

int do_read (const char *path, char *buffer, size_t size, off_t offset,
             struct fuse_file_info *)
{
    try
    {
        std::string parsed_path;
        snapshot_ver_t version = if_snapshot(path, parsed_path);
        auto inode_id = filesystem_inode_smi->get_inode_id_by_path(path);
        auto inode = filesystem_inode_smi->get_inode_by_id(inode_id);
        inode->fs_stat.st_atim = get_current_time();
        return (int)inode->read(version, buffer, size, offset);
    }
    CATCH_TAIL;
}

int do_write (const char * path, const char * buffer, size_t size, off_t offset,
              struct fuse_file_info * info)
{
    try
    {
        CHECK_RDONLY_FS(path);

        auto inode_id = filesystem_inode_smi->get_inode_id_by_path(path);
        auto inode = filesystem_inode_smi->get_inode_by_id(inode_id);
        auto current_data_sz = inode->current_data_size(FILESYSTEM_CUR_MODIFIABLE_VER);
        inode->fs_stat.st_mtim = get_current_time();
        bool if_resize = false;
        if ((offset + size) > current_data_sz)
        {
            if_resize = true;
        }

        return (int)inode->write(buffer, size, offset, if_resize);
    }
    CATCH_TAIL;
}

int do_utimens (const char * path, const struct timespec tv[2])
{
    try
    {
        CHECK_RDONLY_FS(path);

        auto inode_id = filesystem_inode_smi->get_inode_id_by_path(path);
        auto inode = filesystem_inode_smi->get_inode_by_id(inode_id);

        inode->fs_stat.st_atim = tv[0];
        inode->fs_stat.st_mtim = tv[1];

        return 0;
    }
    CATCH_TAIL;
}

int do_unlink (const char * path)
{
    try
    {
        CHECK_RDONLY_FS(path);

        filesystem_inode_smi->remove_inode_by_path(path);

        return 0;
    }
    CATCH_TAIL;
}

int do_rmdir (const char * path)
{
    try
    {
        path_t vpath(path);
        std::string target_name = vpath.pop_end();

        if (target_name.empty())
        {
            THROW_HTMPFS_ERROR_STDERR(HTMPFS_INVALID_DENTRY_NAME);
        }

        if (target_name == SNAPSHOT_ENTRY)
        {
            return -EROFS;
        }

        // now, determine if creating snapshot volume or normal directory
        if (vpath.size() == 2 /* {"", ".snapshot"} */ &&
            vpath.last()->length() == strlen(SNAPSHOT_ENTRY) &&
            !memcmp(vpath.last()->c_str(),
                    SNAPSHOT_ENTRY,vpath.last()->length())
                )
        {
            // it's a snapshot deletion
            filesystem_inode_smi->delete_snapshot_volume(target_name);
        }
        else
        {
            CHECK_RDONLY_FS(path);
            filesystem_inode_smi->remove_inode_by_path(path);
        }

        return 0;
    }
    CATCH_TAIL;
}

int do_fsync (const char * path, int, struct fuse_file_info *)
{
    return 0;
}

int do_releasedir (const char * path, struct fuse_file_info *)
{
    return 0;
}

int do_fsyncdir (const char * path, int, struct fuse_file_info *)
{
    return 0;
}

int do_truncate (const char * path, off_t size)
{
    try
    {
        CHECK_RDONLY_FS(path);

        inode_t * inode;

        // create new inode if not exist
        try {
            __disable_output = true;
            inode = filesystem_inode_smi->get_inode_by_id(
                    filesystem_inode_smi->get_inode_id_by_path(path));
        }
        catch (HTMPFS_error_t & error)
        {
            __disable_output = false;

            // if inode doesn't exist
            if (error.my_errcode() == HTMPFS_NO_SUCH_FILE_OR_DIR)
            {
                path_t vpath(path);
                auto target_name = vpath.pop_end();

                if (vpath.size() == 0)
                {
                    THROW_HTMPFS_ERROR_STDERR(HTMPFS_INVALID_DENTRY_NAME);
                }

                auto inode_id = filesystem_inode_smi->get_inode_id_by_path(vpath.to_string());
                auto new_inode_id = filesystem_inode_smi->
                        make_child_dentry_under_parent(inode_id, target_name);
                inode = filesystem_inode_smi->get_inode_by_id(new_inode_id);

                // fill up info
                auto cur_time = get_current_time();
                inode->fs_stat.st_size = size;
                inode->fs_stat.st_nlink = 1;
                inode->fs_stat.st_atim = cur_time;
                inode->fs_stat.st_ctim = cur_time;
                inode->fs_stat.st_mtim = cur_time;
            }
        }

        __disable_output = false;

        // resize inode buffer
        inode->truncate(size);

        return 0;
    }
    CATCH_TAIL;
}

int do_symlink  (const char * path, const char * target)
{
    try
    {
        CHECK_RDONLY_FS(path);
        CHECK_RDONLY_FS(target);

        path_t vpath(target);
        auto target_name = vpath.pop_end();
        
        // invalid target name
        if (target_name.empty())
        {
            THROW_HTMPFS_ERROR_STDERR(HTMPFS_INVALID_DENTRY_NAME);
        }

        auto parent_inode = filesystem_inode_smi->get_inode_id_by_path(vpath.to_string());
        auto new_inode_id = filesystem_inode_smi->
                make_child_dentry_under_parent(parent_inode, target_name);
        auto new_inode = filesystem_inode_smi->get_inode_by_id(new_inode_id);
        
        // fill up info
        auto cur_time = get_current_time();
        new_inode->fs_stat.st_mode = S_IFLNK | 0755;
        new_inode->fs_stat.st_nlink = 1;
        new_inode->fs_stat.st_atim = cur_time;
        new_inode->fs_stat.st_ctim = cur_time;
        new_inode->fs_stat.st_mtim = cur_time;
        new_inode->fs_stat.st_size = (off_t)strlen(target);
        new_inode->write(target, strlen(target), 0);

        return 0;
    }
    CATCH_TAIL;
}

int do_rename (const char * path, const char * name)
{
    try
    {
        CHECK_RDONLY_FS(path);
        CHECK_RDONLY_FS(name);

        auto inode_id = filesystem_inode_smi->get_inode_id_by_path(path);

        // get target name/path
        path_t original(path);
        auto original_name = original.pop_end();
        if (original.size() == 0) {
            THROW_HTMPFS_ERROR_STDERR(HTMPFS_INVALID_DENTRY_NAME);
        }

        // get original name/path
        path_t target(name);
        auto target_name = target.pop_end();
        if (target.size() == 0) {
            THROW_HTMPFS_ERROR_STDERR(HTMPFS_INVALID_DENTRY_NAME);
        }

        // first, remove dentry in parent inode
        auto parent_inode_id = filesystem_inode_smi->get_inode_id_by_path(original.to_string());
        auto parent_inode = filesystem_inode_smi->get_inode_by_id(parent_inode_id);
        directory_resolver_t ori_directoryResolver(parent_inode, FILESYSTEM_CUR_MODIFIABLE_VER);
        ori_directoryResolver.remove_path(original_name);
        ori_directoryResolver.save_current();

        // second, create new dentry in new inode parent
        auto target_parent_inode_id = filesystem_inode_smi->get_inode_id_by_path(target.to_string());
        auto target_parent_inode = filesystem_inode_smi->get_inode_by_id(target_parent_inode_id);
        directory_resolver_t tag_directoryResolver(target_parent_inode,
                                                   FILESYSTEM_CUR_MODIFIABLE_VER);
        tag_directoryResolver.add_path(target_name, inode_id);
        tag_directoryResolver.save_current();


        return 0;

    }
    CATCH_TAIL;
}

int do_fallocate(const char * path, int mode, off_t offset, off_t length, struct fuse_file_info *)
{
    try
    {
        CHECK_RDONLY_FS(path);

        auto inode_id = filesystem_inode_smi->get_inode_id_by_path(path);
        auto inode = filesystem_inode_smi->get_inode_by_id(inode_id);

        // fill up info
        auto cur_time = get_current_time();
        inode->fs_stat.st_mode = mode | S_IFREG;
        inode->fs_stat.st_nlink = 1;
        inode->fs_stat.st_ctim = cur_time;
        inode->fs_stat.st_size = offset + length;

        // resize
        inode->truncate(offset + length);

        return 0;
    }
    CATCH_TAIL;
}

int do_fgetattr (const char * path, struct stat * statbuf, struct fuse_file_info *)
{
    return do_getattr(path, statbuf);
}

int do_ftruncate (const char * path, off_t length, struct fuse_file_info *)
{
    return do_truncate(path, length);
}

int do_readlink (const char * path, char * buffer, size_t size)
{
    try
    {
        std::string parsed_path;
        snapshot_ver_t version = if_snapshot(path, parsed_path);
        auto inode_id = filesystem_inode_smi->get_inode_id_by_path(path);
        auto inode = filesystem_inode_smi->get_inode_by_id(inode_id);
        if (inode->fs_stat.st_mode & S_IFLNK) {
            inode->read(version, buffer, size, 0);
        } else {
            return -EINVAL;
        }

        return 0;
    }
    CATCH_TAIL;
}

void do_destroy (void *)
{
    __asm__("nop");
}

void* do_init (struct fuse_conn_info *conn)
{
    conn->capable |= FUSE_CAP_ATOMIC_O_TRUNC;
    return nullptr;
}

/// dereference inode
/// @param root_inode root inode id
/// @param version snapshot volume version
/// @return next pathname
inline std::string dereference_inode(inode_id_t root_inode,
                         const snapshot_ver_t& version)
{
    try {
        auto inode = filesystem_inode_smi->get_inode_by_id(root_inode);
        inode->to_string(version);
    } catch (...) {
        throw;
    }
}

int do_mknod (const char * path, mode_t mode, dev_t device)
{
    try
    {
        CHECK_RDONLY_FS(path);

        path_t vpath(path);
        auto target_name = vpath.pop_end();
        if (target_name.empty()) {
            THROW_HTMPFS_ERROR_STDERR(HTMPFS_INVALID_DENTRY_NAME);
        }
        
        auto inode_id = filesystem_inode_smi->get_inode_id_by_path(vpath.to_string());
        auto new_inode_id = filesystem_inode_smi->
                make_child_dentry_under_parent(inode_id, target_name);
        auto new_inode = filesystem_inode_smi->get_inode_by_id(new_inode_id);

        // fill up info
        auto cur_time = get_current_time();
        new_inode->fs_stat.st_mode  = mode;
        new_inode->fs_stat.st_nlink = 1;
        new_inode->fs_stat.st_atim  = cur_time;
        new_inode->fs_stat.st_ctim  = cur_time;
        new_inode->fs_stat.st_mtim  = cur_time;
        new_inode->fs_stat.st_dev   = device;

        return 0;
    }
    CATCH_TAIL;
}
