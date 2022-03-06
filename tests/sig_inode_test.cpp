#include <debug.h>
#include <iostream>
#include <htmpfs.h>
#include <string>

/** @file
 *
 * This file handles test for single inode
 */

#define VERIFY_DATA_OPS_LEN(operation, len) if ((operation) != len) { return EXIT_FAILURE; } __asm__("nop")
#define VERIFY_DATA(ops, data) if ((ops).to_string(0) != (data)) { return EXIT_FAILURE; } __asm__("nop")
#define VERIFY_DATA_BARE(ops, data) if (std::string(ops) != std::string(data)) { return EXIT_FAILURE; } __asm__("nop")

int main(int argc, char ** argv)
{
    inode_smi_t filesystem(2);

    {
        /// instance 1: bare write, resize enabled, no offset
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        VERIFY_DATA(inode, "123456789");
    }

    {
        /// instance 2: bare write, extended, resize enabled, with offset
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        VERIFY_DATA_OPS_LEN(inode.write("10", 2, 8), 2);
        VERIFY_DATA(inode, "1234567810");
    }

    {
        /// instance 3: bare write, shortage, resize enabled, without offset
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        VERIFY_DATA_OPS_LEN(inode.write("10", 2, 0), 2);
        VERIFY_DATA(inode, "10");
    }

    {
        /// instance 4: bare write, shortage, resize enabled, with offset
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        VERIFY_DATA_OPS_LEN(inode.write("10", 2, 2), 2);
        VERIFY_DATA(inode, "1210");
    }

    {
        /// instance 5: bare write, shortage, resize disabled, without offset
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        VERIFY_DATA_OPS_LEN(inode.write("10", 2, 0, false), 2);
        VERIFY_DATA(inode, "103456789");
    }

    {
        /// instance 6: bare write, shortage, resize disabled, with offset
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        VERIFY_DATA_OPS_LEN(inode.write("10", 2, 2, false), 2);
        VERIFY_DATA(inode, "121056789");
    }

    {
        /// instance 7: bare write, extended, resize disabled, without offset
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        VERIFY_DATA_OPS_LEN(inode.write("0000000123456", 13, 0, false), 9);
        VERIFY_DATA(inode, "000000012");
    }

    {
        /// instance 8: bare write, extended, resize disabled, with offset
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        VERIFY_DATA_OPS_LEN(inode.write("12345678910", 11, 5, false), 4);
        VERIFY_DATA(inode, "123451234");
    }

    {
        /// instance 9: bare write, offset > bank size, resize disabled
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 12, false), 0);
    }

    {
        /// instance 10: bare read
        inode_t inode(2, 0, &filesystem);
        char buff[512]{};
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        VERIFY_DATA_OPS_LEN(inode.read(0, buff, 9, 0), 9);
        VERIFY_DATA_BARE(buff, "123456789");
    }

    {
        /// instance 11: bare read, bank size shortage, without offset
        inode_t inode(2, 0, &filesystem);
        char buff[512]{};
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        VERIFY_DATA_OPS_LEN(inode.read(0, buff, sizeof(buff), 0), 9);
        VERIFY_DATA_BARE(buff, "123456789");
    }

    {
        /// instance 11: bare read, bank size shortage, with offset
        inode_t inode(2, 0, &filesystem);
        char buff[512]{};
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        VERIFY_DATA_OPS_LEN(inode.read(0, buff, sizeof(buff), 3), 6);
        VERIFY_DATA_BARE(buff, "456789");
    }

    {
        /// instance 12: bare read, offset > bank size
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.read(0, nullptr, 9, 12), 0);
    }

    return EXIT_SUCCESS;
}
