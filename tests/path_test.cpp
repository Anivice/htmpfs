#include <path_t.h>
#include <string>
#include <iostream>

/** @file
 *
 * This file handles test for pathname resolver
 */


int main(int argc, char ** argv)
{
    path_t path("/usr/bin/bash");

    for (const auto& i : path)
    {
        std::cout << i << " ";
    }
    std::cout << path.size() << " ";

    path_t path2("/");

    for (const auto& i : path2)
    {
        std::cout << path2.size();
    }

    return EXIT_SUCCESS;
}
