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
    directory_resolver_t directory_resolver(
            "/usr/var/bin/etc/dev/boot/lib/lib64/sys/tmp/proc",
            nullptr);
    std::cout << directory_resolver.to_string() << std::endl;
    auto vec = directory_resolver.to_vector();

    for (const auto& i : vec)
    {
        std::cout << i << " ";
    }

    std::cout << std::endl;

    directory_resolver.add_path("lost+found");

    std::cout << directory_resolver.to_string() << std::endl;

    return EXIT_SUCCESS;
}
