#include <htmpfs_error.h>
#include <iostream>

/** @file
 *
 * This file defines test for error class
 */

int main()
{
    try {
        THROW_HTMPFS_ERROR_STDERR(HTMPFS_SUCCESSFUL);
    }
    catch (std::exception & err)
    {
        std::cerr << err.what() << std::endl;
    }

    try {
        THROW_HTMPFS_ERROR(1, HTMPFS_SUCCESSFUL, "Nothing to show here");
    }
    catch (std::exception & err)
    {
        std::cout << err.what() << std::endl;
    }

    try {
        THROW_HTMPFS_ERROR(2);
    }
    catch (std::exception & err)
    {
        std::cerr << err.what() << std::endl;
    }

    return 0;
}
