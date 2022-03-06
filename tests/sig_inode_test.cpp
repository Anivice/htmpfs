#include <debug.h>
#include <iostream>
#include <htmpfs.h>
#include <map>

/** @file
 *
 * This file handles test for single inode
 */

int main(int argc, char ** argv)
{
    std::map < int, int > map;

    for (int i = 0; i < 10; i++)
    {
        map.emplace(i, i);
    }

    auto m = map.size();

    inode_t inode(2, 0, nullptr);
    inode.write("123456789", 9, 0);
    inode.write("1", 3, 8);
    inode.write("0", 1, 4);

    inode.write("123456789", 9, 0, false);

    char buffer[16] { };

    inode.read(0, buffer, sizeof(buffer), 3);

    std::cout << inode.to_string(0) << std::endl;
}
