#include <path_t.h>
#include <string>
#include <cstring>
#include <iostream>

/** @file
 *
 * This file handles test for custom iterator
 */


int main(int argc, char ** argv)
{
    // initialize an iterator
    _custom_it_t < int > iterator;
    *iterator = 0;

    for (int i = 1; i < 32; i++)
    {
        iterator.append(i);
    }

    {
        /// instance 1: traditional C for() loop

        int count = 0;
        for (auto i = iterator.begin(); i != iterator.end(); i++)
        {
            if (count != *i)
            {
                return EXIT_FAILURE;
            }

            count++;
            std::cout << *i << " ";
        }

        if (count != iterator.size())
        {
            return EXIT_FAILURE;
        }

        std::cout << std::endl;
    }

    {
        /// instance 2: C++ 11 for() loop

        int count = 0;
        for (auto i : iterator)
        {
            if (count != i)
            {
                return EXIT_FAILURE;
            }

            count++;
            std::cout << i << " ";
        }

        if (count != iterator.size())
        {
            return EXIT_FAILURE;
        }

        std::cout << std::endl;
    }

    {
        /// instance 3: invalid access

        try
        {
            *iterator.end();
            return EXIT_FAILURE;
        }
        catch (...)
        {
        }
    }

    return EXIT_SUCCESS;
}
