#ifndef HTMPFS_FUSE_H
#define HTMPFS_FUSE_H

#define FUSE_USE_VERSION 31
#include <fuse.h>
#include <fuse_ops.h>
#include <sys/types.h>
#include <htmpfs/path_t.h>
#include <htmpfs/htmpfs.h>
#include <htmpfs_error.h>
#include <iostream>
#include <execinfo.h>
#include <sys/xattr.h>
#include <htmpfs/htmpfs.h>

// A generic smart pointer class
template < class Type >
class SmartPtr {
    Type* ptr = nullptr; // Actual pointer
public:
    void set(Type * _val) { ptr = _val; }
    ~SmartPtr() { delete (ptr); }
    Type& operator*() { return *ptr; }
    Type* operator->() { return ptr; }
};

extern SmartPtr < inode_smi_t > filesystem_inode_smi;

int do_getattr  (const char * path, struct stat *stbuf);
int do_readlink (const char * path, char *, size_t);
int do_mknod    (const char * path, mode_t mode, dev_t device);
int do_mkdir    (const char * path, mode_t mode);
int do_unlink   (const char * path);
int do_rmdir    (const char * path);
int do_symlink  (const char * path, const char *);
int do_rename   (const char * path, const char * name);
int do_chmod    (const char * path, mode_t mode);
int do_chown    (const char * path, uid_t uid, gid_t gid);
int do_truncate (const char * path, off_t size);
int do_open     (const char * path, struct fuse_file_info * fi);
int do_read     (const char * path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi);
int do_write    (const char * path, const char * buffer, size_t size, off_t offset, struct fuse_file_info * fi);
int do_flush    (const char * path, struct fuse_file_info * fi);
int do_release  (const char * path, struct fuse_file_info * fi);
int do_fsync    (const char * path, int, struct fuse_file_info *);
int do_readdir  (const char * path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
int do_releasedir (const char *path, struct fuse_file_info *);
int do_fsyncdir (const char * path, int, struct fuse_file_info *);
int do_create   (const char * path, mode_t mode, struct fuse_file_info * fi);
int do_utimens  (const char * path, const struct timespec tv[2]);
int do_fallocate(const char * path, int mode, off_t offset, off_t length, struct fuse_file_info * fi);
int do_access   (const char *, int);
int do_fgetattr (const char *, struct stat *, struct fuse_file_info *);
int do_ftruncate (const char *, off_t, struct fuse_file_info *);
void do_destroy (void *);
void* do_init    (struct fuse_conn_info *conn);

#endif //HTMPFS_FUSE_H
