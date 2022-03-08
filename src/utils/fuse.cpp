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
            !memcmp(vpath.last()->c_str(),
                    SNAPSHOT_ENTRY,
                    MIN(strlen(SNAPSHOT_ENTRY), vpath.last()->length())
                    )
                )
        {
            // it's a snapshot creation
            filesystem_inode_smi->create_snapshot_volume(target_name);
        }
        else
        {
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
        auto inode_id = filesystem_inode_smi->get_inode_id_by_path(path);
        auto inode = filesystem_inode_smi->get_inode_by_id(inode_id);
        inode->fs_stat.st_mode = mode;

        return 0;
    }
    CATCH_TAIL;
}

int do_chown (const char * path, uid_t uid, gid_t gid)
{
    try
    {
        auto inode_id = filesystem_inode_smi->get_inode_id_by_path(path);
        auto inode = filesystem_inode_smi->get_inode_by_id(inode_id);
        inode->fs_stat.st_uid = uid;
        inode->fs_stat.st_gid = gid;

        return 0;
    }
    CATCH_TAIL;
}

int do_create (const char * path, mode_t mode, struct fuse_file_info *)
{
    try
    {
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

int do_open (const char * path, struct fuse_file_info *)
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
        return (int)inode->read(version, buffer, size, offset);
    }
    CATCH_TAIL;
}

int do_write (const char * path, const char * buffer, size_t size, off_t offset,
              struct fuse_file_info *)
{
    try
    {
        auto inode_id = filesystem_inode_smi->get_inode_id_by_path(path);
        auto inode = filesystem_inode_smi->get_inode_by_id(inode_id);
        return (int)inode->write(buffer, size, offset);
    }
    CATCH_TAIL;
}

int do_utimens (const char * path, const struct timespec tv[2])
{
    try
    {
        std::string parsed_path;
        snapshot_ver_t version = if_snapshot(path, parsed_path);
        auto inode_id = filesystem_inode_smi->get_inode_id_by_path(path);
        auto inode = filesystem_inode_smi->get_inode_by_id(inode_id);

        if (version != FILESYSTEM_CUR_MODIFIABLE_VER)
        {
            return -EROFS;  // Read-only filesystem (POSIX.1-2001)
        }
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
        std::string parsed_path;
        snapshot_ver_t version = if_snapshot(path, parsed_path);

        if (version != FILESYSTEM_CUR_MODIFIABLE_VER)
        {
            return -EROFS;  // Read-only filesystem (POSIX.1-2001)
        }

        filesystem_inode_smi->remove_inode_by_path(path);

        return 0;
    }
    CATCH_TAIL;
}

int do_rmdir (const char * path)
{
    try
    {
        std::string parsed_path;
        snapshot_ver_t version = if_snapshot(path, parsed_path);

        if (version != FILESYSTEM_CUR_MODIFIABLE_VER)
        {
            return -EROFS;  // Read-only filesystem (POSIX.1-2001)
        }

        filesystem_inode_smi->remove_inode_by_path(path);

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
        inode_id_t new_inode_id;

        try
        {
            __disable_output = true;
            new_inode_id = filesystem_inode_smi->get_inode_id_by_path(path);
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
                new_inode_id = filesystem_inode_smi->make_child_dentry_under_parent(inode_id,
                                                                                          target_name);
            }
        }

        __disable_output = false;

        auto new_inode = filesystem_inode_smi->get_inode_by_id(new_inode_id);

        // fill up info
        auto cur_time = get_current_time();
        new_inode->fs_stat.st_size = size;
        new_inode->fs_stat.st_nlink = 1;
        new_inode->fs_stat.st_atim = cur_time;
        new_inode->fs_stat.st_ctim = cur_time;
        new_inode->fs_stat.st_mtim = cur_time;

        // resize inode buffer
        new_inode->write(nullptr, size, 0, true, true);

        return 0;
    }
    CATCH_TAIL;
}
