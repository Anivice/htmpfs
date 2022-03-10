#include <debug.h>
#include <iostream>
#include <htmpfs/htmpfs.h>
#include <string>

/** @file
 *
 * This file handles test for single inode
 */

#define VERIFY_DATA_OPS_LEN(operation, len) if ((operation) != len) { return EXIT_FAILURE; } __asm__("nop")
#define VERIFY_DATA(ops, data) if ((ops).to_string(FILESYSTEM_CUR_MODIFIABLE_VER) != (data)) { return EXIT_FAILURE; } __asm__("nop")
#define VERIFY_DATA_BARE(ops, data) if (std::string(ops) != std::string(data)) { return EXIT_FAILURE; } __asm__("nop")
#define VERIFY_DATA_VER(ops, ver, data) if ((ops).to_string(ver) != (data)) { return EXIT_FAILURE; } __asm__("nop")

int main(int argc, char ** argv)
{
    inode_smi_t filesystem(2);

    {
        /// instance 1: bare write, resize enabled, no offset, no snapshot

        INSTANCE("INODE: instance 1: bare write, resize enabled, no offset, no snapshot");
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        VERIFY_DATA(inode, "123456789");
    }

    {
        /// instance 2: bare write, resize enabled, no offset, check snapshot

        INSTANCE("INODE: instance 2: bare write, resize enabled, no offset, check snapshot");
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        inode.create_new_volume("1");
        VERIFY_DATA_VER(inode, "1", "123456789");
    }

    {
        /// instance 3: bare write, resize enabled, no offset, enable snapshot, modify root

        INSTANCE("INODE: instance 3: bare write, resize enabled, no offset, enable snapshot, modify root");
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        inode.create_new_volume("1");
        VERIFY_DATA_OPS_LEN(inode.write("10", 2, 8), 2);

        VERIFY_DATA_VER(inode, "1", "123456789");
        VERIFY_DATA(inode, "1234567810");
    }

    {
        /// instance 4: bare write, resize enabled, with offset, enable snapshot multiple times, modify root

        INSTANCE("INODE: instance 4: bare write, resize enabled, with offset, enable snapshot multiple times, modify root");
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        inode.create_new_volume("1");
        VERIFY_DATA_OPS_LEN(inode.write("10", 2, 8), 2);
        inode.create_new_volume("2");
        VERIFY_DATA_OPS_LEN(inode.write("10", 2, 2), 2);

        VERIFY_DATA_VER(inode, "1", "123456789");
        VERIFY_DATA_VER(inode, "2", "1234567810");
        VERIFY_DATA(inode, "1210");
    }

    {
        /// instance 5: bare write, resize enabled, with offset, snapshot causes grow size

        INSTANCE("INODE: instance 5: bare write, resize enabled, with offset, snapshot causes grow size");
        inode_t inode(2, 0, &filesystem);

        inode.write("1", 1, 0);
        inode.create_new_volume("1");

        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 1), 9);

        VERIFY_DATA_VER(inode, "1", "1");
        VERIFY_DATA(inode, "1123456789");
    }

    {
        /// instance 6: bare write, shortage, resize enabled, with offset

        INSTANCE("INODE: instance 6: bare write, shortage, resize enabled, with offset");
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        inode.create_new_volume("1");
        VERIFY_DATA_OPS_LEN(inode.write("10", 2, 2), 2);
        inode.create_new_volume("2");
        VERIFY_DATA_VER(inode, "1", "123456789");
        VERIFY_DATA_VER(inode, "2", "1210");
        VERIFY_DATA(inode, "1210");
    }

    {
        /// instance 7: bare write, shortage, resize enabled, with offset, middle modify

        INSTANCE("INODE: instance 7: bare write, shortage, resize enabled, with offset, middle modify");
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        inode.create_new_volume("1");
        VERIFY_DATA_OPS_LEN(inode.write("987654321", 9, 0), 9);
        VERIFY_DATA_VER(inode, "1", "123456789");
        VERIFY_DATA_OPS_LEN(inode.write("12", 2, 2), 2);
        VERIFY_DATA(inode, "9812");
    }

    {
        /// instance 8: bare write, shortage, resize disabled, without offset

        INSTANCE("INODE: instance 8: bare write, shortage, resize disabled, without offset");
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        inode.create_new_volume("1");
        VERIFY_DATA_OPS_LEN(inode.write("10", 2, 0, false), 2);
        VERIFY_DATA(inode, "103456789");
        VERIFY_DATA_VER(inode, "1", "123456789");
    }

    {
        /// instance 9: bare write, extended, resize disabled, without offset

        INSTANCE("INODE: instance 9: bare write, extended, resize disabled, without offset");
        inode_t inode(2, 0, &filesystem);
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        inode.create_new_volume("1");
        VERIFY_DATA_OPS_LEN(inode.write("0000000123456", 13, 0, false), 9);
        VERIFY_DATA(inode, "000000012");
    }

    {
        /// instance 10: bare read, bank size shortage, with offset

        INSTANCE("INODE: instance 10: bare read, bank size shortage, with offset");
        inode_t inode(2, 0, &filesystem);
        char buff[512]{};
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        inode.create_new_volume("1");
        VERIFY_DATA_OPS_LEN(inode.read("1", buff, sizeof(buff), 3), 6);
        VERIFY_DATA_BARE(buff, "456789");
    }

    {
        /// instance 11: bare read, bank size shortage, with offset

        INSTANCE("INODE: instance 11: bare read, bank size shortage, with offset");
        inode_t inode(2, 0, &filesystem);
        char buff[512]{};
        VERIFY_DATA_OPS_LEN(inode.write("123456789", 9, 0), 9);
        inode.create_new_volume("1");
        memset(buff, 0, sizeof(buff));
        VERIFY_DATA_OPS_LEN(inode.read("1", buff, sizeof(buff), 3), 6);
        VERIFY_DATA_BARE(buff, "456789");

        VERIFY_DATA_OPS_LEN(inode.write("123", 3, 1, true), 3);
        inode.create_new_volume("2");
        inode.write(nullptr, 0, 0);
        memset(buff, 0, sizeof(buff));
        VERIFY_DATA_OPS_LEN(inode.read("2", buff, sizeof(buff), 3), 1);
        VERIFY_DATA_BARE(buff, "3");
    }

    {
        /// instance 12: bare write, resize enabled, no offset, delete snapshot volume

        INSTANCE("INODE: instance 12: bare write, resize enabled, no offset, delete snapshot volume");
        inode_t inode(2, 0, &filesystem);

        inode.write("123456789", 9, 0);
        inode.create_new_volume("1");

        inode.write("123", 3, 3);
        inode.create_new_volume("2");

        inode.delete_volume("1");

        VERIFY_DATA_VER(inode, "2", "123123");

        try {
            VERIFY_DATA_VER(inode, "1", "123123");
        } catch (HTMPFS_error_t & err)
        {
            if (err.my_errcode() != HTMPFS_NO_SUCH_SNAPSHOT)
            {
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}
