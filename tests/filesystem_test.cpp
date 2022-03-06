#include <htmpfs.h>
#include <iostream>

/** @file
 *
 * This file defines test for filesystem functionality
 */

int main()
{
    inode_smi_t filesystem(4);

    auto id_etc = filesystem.make_child_dentry_under_parent(
            FILESYSTEM_ROOT_INODE_NUMBER, "etc", true);
    auto id_ssh = filesystem.make_child_dentry_under_parent(
            id_etc, "ssh");

    filesystem.get_inode_by_id(id_ssh)->write("123", 3, 0);

    std::cout << filesystem.get_inode_by_id(
            filesystem.get_inode_by_path("/etc/ssh", 0)
            )->to_string(0)
            << std::endl;

    filesystem.remove_child_dentry_under_parent(id_etc, "ssh");
    filesystem.remove_child_dentry_under_parent(FILESYSTEM_ROOT_INODE_NUMBER, "etc");

    return EXIT_SUCCESS;
}
