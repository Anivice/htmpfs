#ifndef HTMPFS_BUFFER_T_H
#define HTMPFS_BUFFER_T_H

#include <cstdint>
#include <vector>
#include <string>
#include <map>

/// universal data type
typedef std::vector <char> data_t;
typedef uint64_t htmpfs_size_t;

class buffer_t
{
private:
    data_t data;

public:
    buffer_t() = default;
    buffer_t(const char *, htmpfs_size_t);

    /// clear buffer
    void clear() { data.clear(); }

    /// get size of buffer
    [[nodiscard]] htmpfs_size_t get() const { return data.size(); }

    /// read(buffer, length, offset)
    /// @param buffer buffer output storage
    /// @param length read length
    /// @param offset read offset
    htmpfs_size_t read(char * buffer, htmpfs_size_t length, htmpfs_size_t offset);

    /// write(buffer, length, offset, resize)
    /// @param buffer write buffer
    /// @param length write length
    /// @param offset write offset
    /// @param resize if resize == true, resize data if length+offset > buffer size
    ///               if resize == false, ignore data beyond buffer size
    htmpfs_size_t write(const char * buffer, htmpfs_size_t length, htmpfs_size_t offset, bool resize = true);

    /// convert to std::string
    std::string to_string();

    /// check if buffer is empty
    [[nodiscard]] bool empty() const { return data.empty(); }

    /// get a hash value for current data bank
    uint64_t hash64();

    /// return size of current data bank
    htmpfs_size_t size() { return data.size(); }
};

#endif //HTMPFS_BUFFER_T_H
