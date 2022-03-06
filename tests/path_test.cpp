#include <path_t.h>
#include <string>
#include <iostream>

/** @file
 *
 * This file handles test for pathname resolver
 */

#define VERIFY_DATA(val, tag) if ((tag) != (val)) { return EXIT_FAILURE; } __asm__("nop")

int main(int argc, char ** argv)
{
    {
        /// instance 1: path match

        path_t path("/usr/bin/bash");
        std::vector < std::string > list ({"", "usr", "bin", "bash"});

        int off = 0;
        for (const auto& i : path)
        {
            VERIFY_DATA(i, list[off++]);
        }
    }

    {
        /// instance 2: empty match

        path_t path2("/");
        VERIFY_DATA(path2.size(), 1);
    }

    return EXIT_SUCCESS;
}
