/** @file
 *
 * This file implements the APIs for FUSE
 */

#define FUSE_USE_VERSION 31
#include <fuse_ops.h>

SmartPtr < inode_smi_t > filesystem_inode_smi;

int do_getattr (const char *path, struct stat *stbuf)
{
    try
    {
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


