#ifndef HTMPFS_LL_IO_H
#define HTMPFS_LL_IO_H

/** @file
 *  this file defines functions for low-level I/O operations
 */

#include <fcntl.h>
#include <sys/types.h>
#include <string>
#include <unistd.h>
#include <htmpfs/htmpfs_types.h>
#include <htmpfs/buffer_t.h>

typedef int fd_t;

class ll_io
{
protected:
    fd_t fd = 0;
    off_t dev_len = 0;

public:
    void    open    (const std::string &);
    off_t   read    (char *, size_t, off_t) const;
    off_t   write   (const char *, size_t, off_t) const;
    void    close   () const    { ::close(fd); }
            ~ll_io  ()          { close(); }
};

class ll_io_blockized : private ll_io
{
private:
    // block size for I/O
    htmpfs_size_t block_size;
    // block count in a device
    htmpfs_size_t block_count;
public:
    /// initiate block I/O device
    ll_io_blockized(const std::string& pathname, htmpfs_size_t _block_size);

    /// read a block
    void read_block(buffer_t & buffer, htmpfs_size_t block_num);

    /// write a block
    void write_block(buffer_t & buffer, htmpfs_size_t block_num);
};


#endif //HTMPFS_LL_IO_H
