#include <directory_resolver.h>
#include <string>
#include <cstring>
#include <iostream>
#include <htmpfs.h>

/** @file
 *
 * This file handles test for directory resolver
 */

#define VERIFY_DATA(val, tag) if ((tag) != (val)) { return EXIT_FAILURE; } __asm__("nop")

int main(int argc, char ** argv)
{
    std::vector < std::string > all_path;
    all_path.emplace_back("usr"); // 0xF0
    all_path.emplace_back("etc"); // 0xF1
    all_path.emplace_back("var"); // 0xF2
    all_path.emplace_back("sys"); // 0xF3
    all_path.emplace_back("lib"); // 0xF4

    {
        /// instance 1: add entries in directory resolver, confined in cache

        INSTANCE("DIR RESOLV: instance 1: add entries in directory resolver, confined in cache");
        inode_smi_t filesystem(2);
        inode_t inode(2, 0, &filesystem, true);
        directory_resolver_t directory_resolver(&inode, 0);

        uint64_t count = 0xF0;
        for (const auto& i : all_path)
        {
            directory_resolver.add_path(i, count++);
        }

        uint64_t off = 0, _count = 0xF0;
        for (const auto & i : directory_resolver)
        {
            VERIFY_DATA(i.pathname, all_path[off++]);
            VERIFY_DATA(i.inode_id, _count++);
        }
    }

    {
        /// instance 2: add entries in directory resolver save to inode

        INSTANCE("DIR RESOLV: instance 2: add entries in directory resolver save to inode");
        inode_smi_t filesystem(2);
        inode_t inode(2, 0, &filesystem, true);
        directory_resolver_t directory_resolver(&inode, 0);

        uint64_t count = 0xF0;
        for (const auto& i : all_path)
        {
            directory_resolver.add_path(i, count++);
        }
        // save changes
        directory_resolver.save_current();

        directory_resolver_t directory_resolver2(&inode, 0);

        uint64_t off = 0, _count = 0xF0;
        for (const auto & i : directory_resolver2)
        {
            VERIFY_DATA(i.pathname, all_path[off++]);
            VERIFY_DATA(i.inode_id, _count++);
        }
    }

    {
        /// instance 3: inode is not a dentry inode

        INSTANCE("DIR RESOLV: instance 3: inode is not a dentry inode");
        try
        {
            inode_smi_t filesystem(2);
            inode_t inode(2, 0, &filesystem, false);
            directory_resolver_t directory_resolver(&inode, 0);
        }
        catch (HTMPFS_error_t & err)
        {
            if (err.my_errcode() != HTMPFS_NOT_A_DIRECTORY)
            {
                return EXIT_FAILURE;
            }
        }
    }

    {
        /// instance 4: export vector

        INSTANCE("DIR RESOLV: instance 4: export vector");
        inode_smi_t filesystem(2);
        inode_t inode(2, 0, &filesystem, true);
        directory_resolver_t directory_resolver(&inode, 0);

        uint64_t count = 0xF0;
        for (const auto& i : all_path)
        {
            directory_resolver.add_path(i, count++);
        }

        auto vec = directory_resolver.to_vector();

        uint64_t off = 0, _count = 0xF0;
        for (const auto & i : vec)
        {
            VERIFY_DATA(i.pathname, all_path[off++]);
            VERIFY_DATA(i.inode_id, _count++);
        }
    }

    {
        /// instance 5: dentry duplicated

        INSTANCE("DIR RESOLV: instance 5: dentry duplicated");
        try
        {
            inode_smi_t filesystem(2);
            inode_t inode(2, 0, &filesystem, true);
            directory_resolver_t directory_resolver(&inode, 0);
            directory_resolver.add_path("dir", 0);
            directory_resolver.add_path("dir", 0);
        }
        catch (HTMPFS_error_t & err)
        {
            if (err.my_errcode() != HTMPFS_DOUBLE_MKPATHNAME)
            {
                return EXIT_FAILURE;
            }
        }
    }

    {
        /// instance 6: makep dentry, both successful and failed

        INSTANCE("DIR RESOLV: instance 6: makep dentry, both successful and failed");
        inode_smi_t filesystem(2);
        inode_t inode(2, 0, &filesystem, true);
        directory_resolver_t directory_resolver(&inode, 0);
        directory_resolver.add_path("dir", 0);

        VERIFY_DATA(directory_resolver.namei("dir"), 0);

        try
        {
            directory_resolver.namei("etc");
        }
        catch (HTMPFS_error_t & err)
        {
            if (err.my_errcode() != HTMPFS_NO_SUCH_FILE_OR_DIR)
            {
                return EXIT_FAILURE;
            }
        }
    }

    {
        /// instance 7: path remove, inter-actively, both successful and failed

        INSTANCE("DIR RESOLV: instance 7: path remove, inter-actively, both successful and failed");
        inode_smi_t filesystem(2);
        inode_t inode(2, 0, &filesystem, true);
        directory_resolver_t directory_resolver(&inode, 0);

        uint64_t count = 0xF0;
        for (const auto& i : all_path)
        {
            directory_resolver.add_path(i, count++);
        }
        // save changes
        directory_resolver.save_current();

        directory_resolver_t directory_resolver2(&inode, 0);
        directory_resolver2.remove_path("etc");
        directory_resolver2.remove_path("sys");
        directory_resolver2.save_current();
        directory_resolver.refresh();

        std::vector < std::string > new_all_path;
        new_all_path.emplace_back("usr"); // 0xF0
        new_all_path.emplace_back("var"); // 0xF2
        new_all_path.emplace_back("lib"); // 0xF4

        std::vector < uint64_t > inode_num_list;
        inode_num_list.emplace_back(0xF0);
        inode_num_list.emplace_back(0xF2);
        inode_num_list.emplace_back(0xF4);

        uint64_t off = 0;
        for (const auto & i : directory_resolver)
        {
            VERIFY_DATA(i.pathname, new_all_path[off]);
            VERIFY_DATA(i.inode_id, inode_num_list[off]);

            off += 1;
        }

        try
        {
            directory_resolver2.remove_path("boot");
        } catch (HTMPFS_error_t & err)
        {
            if (err.my_errcode() != HTMPFS_REQUESTED_INODE_NOT_FOUND)
            {
                return EXIT_FAILURE;
            }
        }
    }

    {
        /// instance 8: check availability, both successful and failed

        INSTANCE("DIR RESOLV: instance 8: check availability, both successful and failed");
        inode_smi_t filesystem(2);
        inode_t inode(2, 0, &filesystem, true);
        directory_resolver_t directory_resolver(&inode, 0);
        directory_resolver.add_path("dir", 0);

        if (!directory_resolver.check_availability("etc"))
        {
            return EXIT_FAILURE;
        }

        if (directory_resolver.check_availability("dir"))
        {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
