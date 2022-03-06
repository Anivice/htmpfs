#ifndef HTMPFS_DIRECTORY_RESOLVER_H
#define HTMPFS_DIRECTORY_RESOLVER_H

#include <string>
#include <vector>
#include <htmpfs.h>

class directory_resolver_t
{
    std::vector < std::string > path;
    inode_t * associated_inode;

public:
    /// create a directory resolver
    explicit directory_resolver_t(const std::string & _path, inode_t * _associated_inode);

    /// make a vector by path
    std::vector < std::string > to_vector();

    /// add path to vector
    void add_path(const std::string &);

    /// make a string by vector
    std::string to_string();
};

#endif //HTMPFS_DIRECTORY_RESOLVER_H
