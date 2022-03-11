/** @file
 *  this file implements functions for loe-level I/O
 */

#include <ll_io/ll_io.h>
#include <htmpfs_error.h>

#define VERIFY_DATA_OPS_LEN(operation, len) \
    if ((operation) != len)                 \
    {                                       \
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_BLOCK_SHORT_OPS); \
    } __asm__("nop")

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

off_t ll_io::read(char * buffer, size_t length, off_t offset) const
{
    if (lseek(fd, offset, SEEK_SET) != offset)
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_CANNOT_LSEEK_DEVICE);
    }

    return ::read(fd, buffer, length);
}

off_t ll_io::write(const char * buffer, size_t length, off_t offset) const
{
    if (lseek(fd, offset, SEEK_SET) != offset)
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_CANNOT_LSEEK_DEVICE);
    }

    return ::write(fd, buffer, length);
}

ll_io_blockized::ll_io_blockized(const std::string& pathname, htmpfs_size_t _block_size)
{
    open(pathname);
    block_size = _block_size;
    block_count = this->dev_len / block_size;
}

void ll_io_blockized::read_block(buffer_t &buffer, htmpfs_size_t block_num)
{
    if (block_num > block_count)
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_MEET_DEVICE_BOUNDARY);
    }
    char * c_buff = new char [block_size];
    VERIFY_DATA_OPS_LEN(read(c_buff, block_size, (off_t)(block_num * block_size)),
                        block_size);
    buffer.write(c_buff, block_size, 0, true);
    delete[] c_buff;
}

void ll_io_blockized::write_block(buffer_t & buffer, htmpfs_size_t block_num)
{
    if (block_num > block_count)
    {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_MEET_DEVICE_BOUNDARY);
    }
    char * c_buff = new char [block_size];
    buffer.read(c_buff, block_size, 0);
    VERIFY_DATA_OPS_LEN(write(c_buff, block_size, (off_t)(block_num * block_size)),
                        block_size);
    delete[] c_buff;
}
