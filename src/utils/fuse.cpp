/** @file
 *
 * This file implements the APIs for FUSE
 */

#define FUSE_USE_VERSION 31
#include <fuse_ops.h>
#include <unistd.h>

SmartPtr < inode_smi_t > filesystem_inode_smi;

int do_getattr (const char *path, struct stat *stbuf)
{
    try
    {
        if (!strcmp("/.snapshot", path)) // non-existing directory
        {
            stbuf->st_ctim  = get_current_time();
            stbuf->st_atim  = get_current_time();
            stbuf->st_mtim  = get_current_time();
            stbuf->st_ino   = 0;
            stbuf->st_mode  = 0766 | S_IFDIR;
            stbuf->st_gid   = getgid();
            stbuf->st_uid   = getuid();
            stbuf->st_nlink = 1;
            stbuf->st_size  = 0;
            return 0;
        }
        
        auto inode_id = filesystem_inode_smi->get_inode_id_by_path(path);
        auto inode = filesystem_inode_smi->get_inode_by_id(inode_id);
        *stbuf = inode->fs_stat;

        return 0;
    }
    catch (std::exception & error)
    {
        std::cerr << error.what() << " (errno=" << strerror(errno) << ")" << std::endl;
        return -errno;
    }
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
            filler(buffer, ".snapshot", nullptr, 0); // .snapshot volume
        }

        if (!strcmp("/.snapshot", path)) // non-existing directory
        {
            for (const auto& i : filesystem_inode_smi->_snapshot_version_list)
            {
                filler(buffer, i.first.c_str(), nullptr, 0);
            }

            return 0;
        }

        auto inode_id = filesystem_inode_smi->get_inode_id_by_path(path);
        auto inode = filesystem_inode_smi->get_inode_by_id(inode_id);
        directory_resolver_t directoryResolver(inode, 0);

        auto dentry_list = directoryResolver.to_vector();
        for (const auto &i: dentry_list)
        {
            filler(buffer, i.pathname.c_str(), nullptr, 0);
        }

        return 0;
    }
    catch (std::exception & error)
    {
        std::cerr << error.what() << " (errno=" << strerror(errno) << ")" << std::endl;
        return -errno;
    }
}
