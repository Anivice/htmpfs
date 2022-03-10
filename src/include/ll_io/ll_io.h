#ifndef HTMPFS_LL_IO_H
#define HTMPFS_LL_IO_H

#include <fcntl.h>
#include <sys/types.h>
#include <string>
#include <unistd.h>

typedef int fd_t;

class ll_io
{
    fd_t fd = 0;
    off_t dev_len = 0;

public:
    void open(const std::string &);
    off_t read(char *, size_t, off_t);
    off_t write(const char *, size_t, off_t);
    void close () { ::close(fd); }
};


#endif //HTMPFS_LL_IO_H
