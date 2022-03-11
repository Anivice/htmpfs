/** @file
 *
 * This file handles test for pathname parser
 */

#include <htmpfs/path_t.h>
#include <string>
#include <iostream>
#include <debug.h>

#define VERIFY_DATA(val, tag) if ((tag) != (val)) { return EXIT_FAILURE; } __asm__("nop")

int main(int argc, char ** argv)
{
    {
        /// instance 1: path match

        INSTANCE("PATH_T: instance 1: path match");
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

        INSTANCE("PATH_T: instance 2: empty match");
        path_t path2("/");
        VERIFY_DATA(path2.size(), 1);
    }

    {
        /// instance 3: path ends with '/'

        INSTANCE("PATH_T: instance 3: path ends with '/'");
        path_t path2("/etc/");
        VERIFY_DATA(path2.size(), 2);
    }

    {
        /// instance 4: pop path

        INSTANCE("PATH_T: instance 4: pop path");
        path_t path2("/etc/target");
        path2.pop_end();
        VERIFY_DATA(path2.to_string(), "/etc");
    }

    return EXIT_SUCCESS;
}
