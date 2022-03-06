#ifndef HTMPFS_DIRECTORY_RESOLVER_H
#define HTMPFS_DIRECTORY_RESOLVER_H

#include <string>
#include <vector>
#include <buffer_t.h>

class inode_t;

class directory_resolver_t
{
    struct path_pack_t
    {
        std::string pathname;
        uint64_t inode_id;
    };
    std::vector < path_pack_t > path;
    inode_t * associated_inode;
    uint64_t access_version;

public:
    /// create a directory resolver
    explicit directory_resolver_t(inode_t * _associated_inode, uint64_t);

    /// make a vector by path
    std::vector < path_pack_t > to_vector();

    /// add path to vector
    void add_path(const std::string &, uint64_t);

    /// save curren dentry string
    void save_current();

    /// get inode id by name
    uint64_t namei(const std::string &);
};

#endif //HTMPFS_DIRECTORY_RESOLVER_H
