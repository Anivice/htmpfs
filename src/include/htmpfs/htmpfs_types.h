#ifndef HTMPFS_HTMPFS_TYPES_H
#define HTMPFS_HTMPFS_TYPES_H

/** @file
 *  this file defines frequently used types
 */

#include <string>
#include <vector>

#define LEFT_SHIFT64(val, bit_count)  ((uint64_t)((uint64_t)(val) << (uint64_t)bit_count))
#define RIGHT_SHIFT64(val, bit_count) ((uint64_t)((uint64_t)(val) >> (uint64_t)bit_count))

#define FILESYSTEM_ROOT_INODE_NUMBER    0x00
#define FILESYSTEM_CUR_MODIFIABLE_VER   "current"

typedef std::string snapshot_ver_t;
typedef uint64_t buffer_id_t;
typedef uint64_t inode_id_t;
struct unique_buffer_pkg_id_t
{
    snapshot_ver_t version;
    buffer_id_t    buffer_id;
    inode_id_t     inode_id;
    uint64_t       pkg_link_count;
};

class inode_smi_t;
class directory_resolver_t;
class buffer_t;
class inode_t;
class bitmap_t;

struct buffer_result_t
{
    buffer_id_t id;
    buffer_t * data;
    uint64_t _is_snapshoted:1;
};

struct inode_result_t
{
    inode_id_t id;
    inode_t * inode;
};

/// universal buffer type
typedef std::vector <char> data_t;
typedef uint64_t htmpfs_size_t;

// A generic smart pointer class
template < class Type >
class SmartPtr {
    Type* ptr = nullptr; // Actual pointer
public:
    void set(Type * _val) { ptr = _val; }
    ~SmartPtr() { delete (ptr); }
    Type& operator*() { return *ptr; }
    Type* operator->() { return ptr; }
};

#endif //HTMPFS_HTMPFS_TYPES_H
