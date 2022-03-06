#include <directory_resolver.h>
#include <string>
#include <cstring>
#include <iostream>

/** @file
 *
 * This file handles test for directory resolver
 */

int main(int argc, char ** argv)
{
    buffer_t buffer;
    directory_resolver_t directory_resolver(buffer, nullptr);
    directory_resolver.add_path("lost+found", 0xF3);
    directory_resolver.add_path("etc", 0xF4);
    directory_resolver.add_path("bin", 0xF5);
    directory_resolver.add_path("usr", 0xF6);
    auto vec = directory_resolver.to_vector();

    for (const auto& i : vec)
    {
        std::cout << i.pathname << " (" << std::hex << i.inode_id << ") ";
    }

    return EXIT_SUCCESS;
}
