#ifndef HTMPFS_DIRECTORY_RESOLVER_H
#define HTMPFS_DIRECTORY_RESOLVER_H

/** @file
 *  this file implements functions for dictionary resolver supporting S_IFDIR inodes
 */

#include <string>
#include <vector>
#include <buffer_t.h>

class inode_t;

class directory_resolver_t
{
public:
    struct path_pack_t
    {
        std::string pathname;
        uint64_t inode_id;
    };

    struct flat_path_pack_t
    {
        char pathname[128] {};
        uint64_t inode_id {};
    };

private:

    std::vector < path_pack_t > path;
    inode_t * associated_inode;
    uint64_t access_version;

    class __dentry_only
    {
    private:
        bool is_dentry_only;

        explicit __dentry_only(bool _is_dentry_only) : is_dentry_only(_is_dentry_only) { };

    public:
        friend directory_resolver_t;
        friend inode_t;
    };

public:
    /// C++ 11 APIs
    std::vector < path_pack_t >::iterator begin() { return path.begin(); }
    std::vector < path_pack_t >::iterator end() { return path.end(); }

    /// create a directory resolver
    /// @param _associated_inode associated inode
    /// @param ver snapshot version
    explicit directory_resolver_t(inode_t * _associated_inode, uint64_t ver);

    /// refresh from inode
    void refresh();

    /// return current target count in current cache
    [[nodiscard]] htmpfs_size_t target_count () const { return path.size(); }

    /// make a vector by path
    /// @return current dentry vector
    std::vector < path_pack_t > to_vector();

    /// add path to vector
    /// @param pathname dentry entry
    /// @param inode_id inode id
    void add_path(const std::string & pathname, uint64_t inode_id);

    /// add path to vector
    /// @param pathname dentry entry
    /// @param inode_id inode id
    void remove_path(const std::string & pathname);

    /// save curren dentry string
    void save_current();

    /// get inode id by name
    /// @param pathname dentry entry
    /// @return inode id
    uint64_t namei(const std::string & pathname);

    /// check if pathname is available
    /// @param pathname pathname pending for checking
    /// @return availability status. true means pathname is available,
    ///         false means pathname is occupied
    bool check_availability(const std::string & pathname);

    friend inode_t;
};

#endif //HTMPFS_DIRECTORY_RESOLVER_H
