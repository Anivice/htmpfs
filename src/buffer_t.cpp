#include <buffer_t.h>
#include <functional>

buffer_t::buffer_t(const char * new_data, htmpfs_size_t length)
{
    for (htmpfs_size_t i = 0; i < length; i++)
    {
        data.emplace_back(new_data[i]);
    }
}

htmpfs_size_t buffer_t::read(char *buffer, htmpfs_size_t length, htmpfs_size_t offset)
{
    htmpfs_size_t read_size;
    if (offset > data.size())
    {
        read_size = 0;
    }
    else if (data.size() < (length + offset))
    {
        read_size = data.size() - offset;
    }
    else
    {
        read_size = length;
    }

    for (htmpfs_size_t i = 0; i < read_size; i++)
    {
        buffer[i] = data[i + offset];
    }

    return read_size;
}

htmpfs_size_t buffer_t::write(const char *buffer, htmpfs_size_t length, htmpfs_size_t offset, bool append)
{
    htmpfs_size_t write_size;
    if (!append)
    {
        if (offset > data.size())
        {
            write_size = 0;
        }
        else if (data.size() < (length + offset))
        {
            write_size = data.size() - offset;
        }
        else
        {
            write_size = length;
        }
    }
    else
    {
        if (length + offset > data.size())
        {
            htmpfs_size_t lost = length + offset - data.size();
            for (htmpfs_size_t i = 0; i < lost; i++)
            {
                data.emplace_back(0);
            }
        }

        write_size = length;
    }

    for (htmpfs_size_t i = 0; i < write_size; i++)
    {
        data[offset + i] = buffer[i];
    }

    return write_size;
}

std::string buffer_t::to_string()
{
    std::string ret(data.begin(), data.end());
    return ret;
}

uint64_t buffer_t::hash64()
{
    return std::hash <std::string>{}(to_string());
}
