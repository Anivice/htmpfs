#include <htmpfs.h>
#include <iostream>

/** @file
 *
 * This file defines test for filesystem functionality
 */

int main()
{
    filesystem_t filesystem(4);

    filesystem.make_child_dentry_under_parent(FILESYSTEM_ROOT_INODE_NUMBER, "etc");
    std::cout << filesystem.get_inode_by_path("/etc", 0) << std::endl;

    return EXIT_SUCCESS;
}
