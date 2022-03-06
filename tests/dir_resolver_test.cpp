#include <directory_resolver.h>
#include <string>
#include <cstring>
#include <iostream>
#include <htmpfs.h>

/** @file
 *
 * This file handles test for directory resolver
 */

int main(int argc, char ** argv)
{
    inode_smi_t filesystem(2);
    inode_t inode(2, 0, &filesystem);
    directory_resolver_t directory_resolver(&inode, 0);
    directory_resolver.add_path("lost+found", 0xF3);
    directory_resolver.add_path("etc", 0xF4);
    directory_resolver.add_path("bin", 0xF5);
    directory_resolver.add_path("usr", 0xF6);
    directory_resolver.save_current();

    directory_resolver_t directory_resolver2(&inode, 0);
    auto vec = directory_resolver2.to_vector();

    for (const auto& i : vec)
    {
        std::cout << i.pathname << " (" << std::hex << i.inode_id << ") " << std::flush;
    }

    directory_resolver.add_path("sys", 0xF7);
    directory_resolver.save_current();

    try {
        std::cout << std::endl << directory_resolver2.namei("sys") << std::endl;
        return EXIT_FAILURE;
    } catch (std::exception & err)
    {
        std::cout << err.what() << std::endl;
    }

    directory_resolver2.refresh();
    std::cout << directory_resolver2.namei("sys") << std::endl;

    if (directory_resolver2.check_availability("sys"))
    {
        return EXIT_FAILURE;
    }

    directory_resolver.remove_path("sys");
    directory_resolver.save_current();
    directory_resolver2.refresh();
    if (!directory_resolver2.check_availability("sys"))
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
