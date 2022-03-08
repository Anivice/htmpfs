#ifndef HTMPFS_HTMPFS_TYPES_H
#define HTMPFS_HTMPFS_TYPES_H

#define LEFT_SHIFT64(val, bit_count)  ((uint64_t)((uint64_t)(val) << (uint64_t)bit_count))
#define RIGHT_SHIFT64(val, bit_count) ((uint64_t)((uint64_t)(val) >> (uint64_t)bit_count))

#define FILESYSTEM_ROOT_INODE_NUMBER    0x00
#define FILESYSTEM_CUR_MODIFIABLE_VER   0x00

typedef uint64_t snapshot_ver_t;
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

#endif //HTMPFS_HTMPFS_TYPES_H
