#!/usr/bin/env bash

# unit check for do_getattr
function check_do_getattr()
{
  mount_point="$1"
  echo "FILESYSTEM CHECK: do_getattr"

  # create 16 files and check availability
  for (( i=0; i < 16; i++ ))
  {
    touch "file_$i"
    ls -lah "$mount_point" > /dev/null
    if [[ $? != 0 ]]; then
      exit 1
    fi
  }

  count=$(ls -lah "$mount_point" | wc -l)
  if [[ "$count" != 19 ]] ; then # 16 files + .snapshot + . + ..
    exit 1
  fi

  echo "FILESYSTEM CHECK: do_getattr clean"
}

# unit check for do_getattr
function check_do_readlink()
{
  mount_point="$1"
  echo "FILESYSTEM CHECK: do_readlink"
  for (( i=0; i < 16; i++ ))
  {
    ls -lah "$mount_point" > /dev/null
    if [[ $? != 0 ]]; then
      exit 1
    fi
  }
  echo "FILESYSTEM CHECK: do_readlink clean"
}

# int do_readlink (const char * path, char *, size_t);
# int do_mknod    (const char * path, mode_t mode, dev_t device);
# int do_mkdir    (const char * path, mode_t mode);
# int do_unlink   (const char * path);
# int do_rmdir    (const char * path);
# int do_symlink  (const char * path, const char *);
# int do_rename   (const char * path, const char * name);
# int do_chmod    (const char * path, mode_t mode);
# int do_chown    (const char * path, uid_t uid, gid_t gid);
# int do_truncate (const char * path, off_t size);
# int do_open     (const char * path, struct fuse_file_info * fi);
# int do_read     (const char * path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi);
# int do_write    (const char * path, const char * buffer, size_t size, off_t offset, struct fuse_file_info * fi);
# int do_flush    (const char * path, struct fuse_file_info * fi);
# int do_release  (const char * path, struct fuse_file_info * fi);
# int do_fsync    (const char * path, int, struct fuse_file_info *);
# int do_readdir  (const char * path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
# int do_releasedir (const char *path, struct fuse_file_info *);
# int do_fsyncdir (const char * path, int, struct fuse_file_info *);
# int do_create   (const char * path, mode_t mode, struct fuse_file_info * fi);
# int do_utimens  (const char * path, const struct timespec tv[2]);
# int do_fallocate(const char * path, int mode, off_t offset, off_t length, struct fuse_file_info * fi);
# int do_fgetattr (const char *, struct stat *, struct fuse_file_info *);
# int do_ftruncate (const char *, off_t, struct fuse_file_info *);

check_do_getattr()
