#include <ll_io/ll_io.h>
#include <htmpfs_error.h>

void ll_io::open(const std::string & pathname)
{
    fd = ::open(pathname.c_str(), O_RDWR | O_DIRECT);
    if (fd == -1)
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_CANNOT_OPEN_DEVICE);
    }

    dev_len = lseek(fd, 0, SEEK_END);

    if (dev_len == -1)
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_CANNOT_LSEEK_DEVICE);
    }
}

off_t ll_io::read(char * buffer, size_t length, off_t offset)
{
    if (lseek(fd, offset, SEEK_SET) != offset)
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_CANNOT_LSEEK_DEVICE);
    }

    return ::read(fd, buffer, length);
}

off_t ll_io::write(const char * buffer, size_t length, off_t offset)
{
    if (lseek(fd, offset, SEEK_SET) != offset)
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_CANNOT_LSEEK_DEVICE);
    }

    return ::write(fd, buffer, length);
}
