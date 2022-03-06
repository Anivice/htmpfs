#ifndef HTMPFS_DIRECTORY_RESOLVER_H
#define HTMPFS_DIRECTORY_RESOLVER_H

#include <string>
#include <vector>
#include <htmpfs.h>
#include <buffer_t.h>

class directory_resolver_t
{
    struct path_pack_t
    {
        std::string pathname;
        uint64_t inode_id;
    };
    std::vector < path_pack_t > path;
    inode_t * associated_inode;

public:
    /// create a directory resolver
    explicit directory_resolver_t(buffer_t & _path, inode_t * _associated_inode);

    /// make a vector by path
    std::vector < path_pack_t > to_vector();

    /// add path to vector
    void add_path(const std::string &, uint64_t);

    /// make a string by vector
    std::vector < char > to_string();
};

#endif //HTMPFS_DIRECTORY_RESOLVER_H
