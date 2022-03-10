/** @file
 *
 * This file implements the mount thread for fuse_main and user-interface
 */

#define FUSE_USE_VERSION 31
#include <fuse.h>
#include <iostream>
#include <fuse_ops.h>
#include <htmpfs.h>
#include <unistd.h>

static struct fuse_operations fuse_ops =
        {
                .getattr    = do_getattr,
                .readlink   = do_readlink,
                .mknod      = do_mknod,
                .mkdir      = do_mkdir,
                .unlink     = do_unlink,
                .rmdir      = do_rmdir,
                .symlink    = do_symlink,
                .rename     = do_rename,
                .chmod      = do_chmod,
                .chown      = do_chown,
                .truncate   = do_truncate,
                .open       = do_open,
                .read       = do_read,
                .write      = do_write,
                .flush      = do_flush,
                .release    = do_release,
                .fsync      = do_fsync,
                .opendir    = do_open,
                .readdir    = do_readdir,
                .releasedir = do_releasedir,
                .fsyncdir   = do_fsyncdir,
                .init       = do_init,
                .destroy    = do_destroy,
                .access     = do_access,
                .create     = do_create,
                .ftruncate  = do_ftruncate,
                .fgetattr   = do_fgetattr,
                .utimens    = do_utimens,
                .fallocate  = do_fallocate,
        };

static void usage(const char *progname)
{
    printf(
            "usage: %s mountpoint [options]\n"
            "\n"
            "general options:\n"
            "    -o opt,[opt...]        Mount options.\n"
            "    -h, --help             Print help.\n"
            "    -V, --version          Print version.\n"
            "\n", progname);
}

enum {
    KEY_VERSION,
    KEY_HELP,
};

static struct fuse_opt fs_opts[] = {
        FUSE_OPT_KEY("-V",              KEY_VERSION),
        FUSE_OPT_KEY("--version",       KEY_VERSION),
        FUSE_OPT_KEY("-h",              KEY_HELP),
        FUSE_OPT_KEY("--help",          KEY_HELP),
        FUSE_OPT_END,
};

static int opt_proc(void *, const char *, int key, struct fuse_args *outargs)
{
    static struct fuse_operations ss_nullptr { };

    switch (key)
    {
        case KEY_VERSION:
            printf("%s Version %s\n", PACKAGE_NAME, PACKAGE_VERSION);
            fuse_opt_add_arg(outargs, "--version");
            fuse_main(outargs->argc, outargs->argv, &ss_nullptr, nullptr);
            fuse_opt_free_args(outargs);
            exit(EXIT_SUCCESS);

        case KEY_HELP:
            usage(outargs->argv[0]);
            fuse_opt_add_arg(outargs, "-ho");
            fuse_main(outargs->argc, outargs->argv, &ss_nullptr, nullptr);
            fuse_opt_free_args(outargs);
            exit(EXIT_SUCCESS);

        default:
            return 1;
    }
}

int main(int argc, char** argv)
{
    try
    {
        // TODO: block size
        filesystem_inode_smi.set(new inode_smi_t(32 * 1024));
        auto *root_inode =
                filesystem_inode_smi->get_inode_by_id(FILESYSTEM_ROOT_INODE_NUMBER);
        // set up root
        root_inode->fs_stat.st_ctim = get_current_time();
        root_inode->fs_stat.st_atim = get_current_time();
        root_inode->fs_stat.st_mtim = get_current_time();
        root_inode->fs_stat.st_ino = FILESYSTEM_ROOT_INODE_NUMBER;
        root_inode->fs_stat.st_mode = 0755 | S_IFDIR;
        root_inode->fs_stat.st_gid = getgid();
        root_inode->fs_stat.st_uid = getuid();
        root_inode->fs_stat.st_nlink = 1;
        root_inode->fs_stat.st_size = 0;

        struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
        if (fuse_opt_parse(&args, nullptr, fs_opts, opt_proc) == -1)
        {
            std::cerr << "FUSE initialization failed, errno: "
                      << strerror(errno) << " (" << errno << ")" << std::endl;
            THROW_HTMPFS_ERROR_STDERR(HTMPFS_EXT_LIB_ERR);
        }

        /*
         * s: run single threaded
         * d: enable debugging
         * f: stay in foreground
         */
        fuse_opt_add_arg(&args, "-s");

#ifdef CMAKE_BUILD_DEBUG
        fuse_opt_add_arg(&args, "-d");
        fuse_opt_add_arg(&args, "-f");
#endif // CMAKE_BUILD_DEBUG

        int ret = fuse_main(args.argc, args.argv, &fuse_ops, nullptr);
        fuse_opt_free_args(&args);

        if (ret != 0)
        {
            THROW_HTMPFS_ERROR_STDERR(HTMPFS_EXT_LIB_ERR);
        }

        return EXIT_SUCCESS;
    }
    catch (std::exception & error)
    {
        std::cerr << error.what() << " (errno=" << strerror(errno) << ")" << std::endl;
        return EXIT_FAILURE;
    }
}
