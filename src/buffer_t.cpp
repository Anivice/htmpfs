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

htmpfs_size_t buffer_t::write(const char *buffer, htmpfs_size_t length, htmpfs_size_t offset, bool resize)
{
    htmpfs_size_t write_size;

    if (!resize)
    {
        if (offset > data.size()) // write beyond buffer
        {
            write_size = 0;
        }
        else if (data.size() < (length + offset)) // write size larger than us, lost some buffer
        {
            write_size = data.size() - offset;
        }
        else // write length OK
        {
            write_size = length;
        }
    }
    else // resize buffer when writing
    {
        if (length + offset > data.size()) // wanted bank is larger than us
        {
            htmpfs_size_t grow_size = length + offset - data.size();
            for (htmpfs_size_t i = 0; i < grow_size; i++)
            {
                data.emplace_back(0);
            }
        }
        else // wanted bank is smaller than us
        {
            htmpfs_size_t lost = data.size() - (length + offset);
            for (htmpfs_size_t i = 0; i < lost; i++)
            {
                data.pop_back();
            }
        }

        write_size = length;
    }

    // write buffer
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
