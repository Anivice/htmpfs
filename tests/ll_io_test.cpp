/** @file
 *
 * This file defines test for low-level I/O
 */

#include <ll_io/ll_io.h>
#include <fstream>
#include <iostream>
#include <cstring>

int main()
{
    {
        /// instance 1: dev I/O
        unlink("tmp");
        std::fstream file("tmp", std::ios::out);
        file.close();

        ll_io dev;
        dev.open("tmp");
        dev.write("FFF", 3, 1);
        char buff[32]{};
        dev.read(buff, 32, 0);
        if (!!memcmp(buff, "\0FFF", 4))
        {
            return EXIT_FAILURE;
        }
    }
}
