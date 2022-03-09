#include <debug.h>
#include <iostream>
#include <htmpfs.h>
#include <string>

/** @file
 *
 * This file handles test for single inode
 */

#define VERIFY_DATA_OPS_LEN(operation, len) if ((operation) != len) { return EXIT_FAILURE; } __asm__("nop")
#define VERIFY_DATA(ops, data) if ((ops).to_string(FILESYSTEM_CUR_MODIFIABLE_VER) != (data)) { return EXIT_FAILURE; } __asm__("nop")
#define VERIFY_DATA_BARE(ops, data) if (std::string(ops) != std::string(data)) { return EXIT_FAILURE; } __asm__("nop")

int main(int argc, char ** argv)
{
    inode_smi_t filesystem(4);

    {
        /// instance 1: bare write, resize enabled, no offset

        INSTANCE("INODE: instance 1: bare write, resize enabled, no offset");
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        VERIFY_DATA(inode, "123456789");
    }

    {
        /// instance 2: bare write, extended, resize enabled, with offset

        INSTANCE("INODE: instance 2: bare write, extended, resize enabled, with offset");
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        VERIFY_DATA_OPS_LEN(inode.write("10", 2, 8), 2);
        VERIFY_DATA(inode, "1234567810");
    }

    {
        /// instance 3: bare write, shortage, resize enabled, without offset

        INSTANCE("INODE: instance 3: bare write, shortage, resize enabled, without offset");
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        VERIFY_DATA_OPS_LEN(inode.write("10", 2, 0), 2);
        VERIFY_DATA(inode, "10");
    }

    {
        /// instance 4: bare write, shortage, resize enabled, with offset

        INSTANCE("INODE: instance 4: bare write, shortage, resize enabled, with offset");
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        VERIFY_DATA_OPS_LEN(inode.write("10", 2, 2), 2);
        VERIFY_DATA(inode, "1210");
    }

    {
        /// instance 5: bare write, shortage, resize disabled, without offset

        INSTANCE("INODE: instance 5: bare write, shortage, resize disabled, without offset");
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        VERIFY_DATA_OPS_LEN(inode.write("10", 2, 0, false), 2);
        VERIFY_DATA(inode, "103456789");
    }

    {
        /// instance 6: bare write, shortage, resize disabled, with offset

        INSTANCE("INODE: instance 6: bare write, shortage, resize disabled, with offset");
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        VERIFY_DATA_OPS_LEN(inode.write("10", 2, 2, false), 2);
        VERIFY_DATA(inode, "121056789");
    }

    {
        /// instance 7: bare write, extended, resize disabled, without offset

        INSTANCE("INODE: instance 7: bare write, extended, resize disabled, without offset");
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        VERIFY_DATA_OPS_LEN(inode.write("0000000123456", 13, 0, false), 9);
        VERIFY_DATA(inode, "000000012");
    }

    {
        /// instance 8: bare write, extended, resize disabled, with offset

        INSTANCE("INODE: instance 8: bare write, extended, resize disabled, with offset");
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        VERIFY_DATA_OPS_LEN(inode.write("12345678910", 11, 5, false), 4);
        VERIFY_DATA(inode, "123451234");
    }

    {
        /// instance 9: bare write, offset > bank size, resize disabled

        INSTANCE("INODE: instance 9: bare write, offset > bank size, resize disabled");
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 12, false), 0);
    }

    {
        /// instance 10: bare read

        INSTANCE("INODE: instance 10: bare read");
        inode_t inode(2, 0, &filesystem);
        char buff[512]{};
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        VERIFY_DATA_OPS_LEN(inode.read(FILESYSTEM_CUR_MODIFIABLE_VER, buff, 9, 0), 9);
        VERIFY_DATA_BARE(buff, "123456789");
    }

    {
        /// instance 11: bare read, bank size shortage, without offset

        INSTANCE("INODE: instance 11: bare read, bank size shortage, without offset");
        inode_t inode(2, 0, &filesystem);
        char buff[512]{};
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        VERIFY_DATA_OPS_LEN(inode.read(FILESYSTEM_CUR_MODIFIABLE_VER, buff, sizeof(buff), 0), 9);
        VERIFY_DATA_BARE(buff, "123456789");
    }

    {
        /// instance 12: bare read, bank size shortage, with offset

        INSTANCE("INODE: instance 12: bare read, bank size shortage, with offset");
        inode_t inode(2, 0, &filesystem);
        char buff[512]{};
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        VERIFY_DATA_OPS_LEN(inode.read(FILESYSTEM_CUR_MODIFIABLE_VER, buff, sizeof(buff), 3), 6);
        VERIFY_DATA_BARE(buff, "456789");
    }

    {
        /// instance 12: bare read, offset > bank size

        INSTANCE("INODE: instance 13: bare read, offset > bank size");
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.read(FILESYSTEM_CUR_MODIFIABLE_VER, nullptr, 9, 12), 0);
    }

    {
        /// instance 14: invalid write access

        INSTANCE("INODE: instance 14: invalid write access");
        inode_t inode(2, 0, &filesystem, true);

        try
        {
            inode.write(nullptr, 0, 0, true);
        }
        catch (HTMPFS_error_t & err)
        {
            if (err.my_errcode() != HTMPFS_INVALID_WRITE_INVOKE)
            {
                return EXIT_FAILURE;
            }
        }
    }

    {
        /// instance 15: resize manually

        inode_smi_t _filesystem(2);
        INSTANCE("INODE: instance 15: resize manually");
        inode_t inode(2, 0, &_filesystem, false);

        inode.write("123456789", 9, 0);
        inode.truncate(3);
        VERIFY_DATA(inode, "123");
        inode.truncate(6);
        if (!!memcmp(inode.to_string(FILESYSTEM_CUR_MODIFIABLE_VER).c_str(), "123\0\0\0", 6))
        {
            return EXIT_FAILURE;
        }
    }

    {
        /// instance 16: resize manually, large bank size

        inode_smi_t _filesystem(32 * 1024);
        INSTANCE("INODE: 16: resize manually, large bank size");
        inode_t inode(32 * 1024, 0, &_filesystem, false);

        inode.write("123456789", 9, 0);
        inode.truncate(3);
        VERIFY_DATA(inode, "123");
        inode.truncate(6);
        if (!!memcmp(inode.to_string(FILESYSTEM_CUR_MODIFIABLE_VER).c_str(), "123\0\0\0", 6))
        {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
