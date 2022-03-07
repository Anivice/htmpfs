#include <htmpfs.h>
#include <iostream>
#include <string>
#include <random>
#include <map>
#include <cmath>
#include <sys/param.h>
#include <cstring>
#include <buffer_t.h>
#include <sstream>

/** @file
 *
 * This file defines test for filesystem functionality
 */

#define VERIFY_DATA(val, tag) if ((tag) != (val)) { return EXIT_FAILURE; } __asm__("nop")

std::string gen_random_data(uint64_t length, uint64_t start = 1, uint64_t end = 255)
{
    std::random_device dev;
    std::string ret;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist255(start,end);

    for (uint64_t i = 0; i < length; i++)
    {
        ret += (char)dist255(rng);
    }

    return ret;
}

std::string gen_random_name(uint64_t length)
{
    buffer_t buffer;
    std::stringstream str;
    buffer.write(gen_random_data(length).c_str(), length, 0);
    str << std::hex << buffer.hash64();
    return str.str();
}

bool can_find(const std::vector < std::string > & list, const std::string & val)
{
    for (const auto & i : list)
    {
        if (!memcmp(i.c_str(), val.c_str(), MIN(i.length(), val.length())))
        {
            return true;
        }
    }

    return false;
}

int main()
{
    {
        /// instance 1: filesystem snapshot, no block change

        // create a new filesystem
        inode_smi_t filesystem(27);
        inode_id_t etc_id =
                filesystem.make_child_dentry_under_parent(FILESYSTEM_ROOT_INODE_NUMBER,
                                                          "etc", true);
        inode_id_t linux_boot =
                filesystem.make_child_dentry_under_parent(FILESYSTEM_ROOT_INODE_NUMBER,
                                                          "linux.boot", false);
        inode_id_t Xorg_conf =
                filesystem.make_child_dentry_under_parent(etc_id, "Xorg.conf", false);

        /* current filesystem layout:
         * /
         * |
         * |---- etc/
         * |        |------Xorg.conf
         * |
         * |---- linux.boot
         * */

        filesystem.remove_child_dentry_under_parent(
                FILESYSTEM_ROOT_INODE_NUMBER,
                "linux.boot");

    }

    {
        /// instance 3: remove an inode, both successful and failed

        // create a new filesystem
        inode_smi_t filesystem(27);
        inode_id_t etc_id =
                filesystem.make_child_dentry_under_parent(FILESYSTEM_ROOT_INODE_NUMBER,
                                                          "etc", true);

        inode_id_t linux_boot =
                filesystem.make_child_dentry_under_parent(FILESYSTEM_ROOT_INODE_NUMBER,
                                                          "linux.boot", false);

        inode_id_t X11 =
                filesystem.make_child_dentry_under_parent(etc_id, "X11", true);
        inode_id_t x11_intel =
                filesystem.make_child_dentry_under_parent(X11, "x11_intel.conf", false);
        inode_id_t Xorg_conf =
                filesystem.make_child_dentry_under_parent(X11, "Xorg.conf", false);

        /* current filesystem layout:
         * /
         * |
         * |---- etc/
         * |        |------X11\
         * |                  |
         * |                  |------- x11_intel.conf
         * |                  |------- Xorg.conf
         * |
         * |---- linux.boot
         * */

        VERIFY_DATA(filesystem.get_inode_id_by_path("/etc", 0), etc_id);
        VERIFY_DATA(filesystem.get_inode_id_by_path("/linux.boot", 0), linux_boot);
        VERIFY_DATA(filesystem.get_inode_id_by_path("/etc/X11", 0), X11);
        VERIFY_DATA(filesystem.get_inode_id_by_path("/etc/X11/x11_intel.conf", 0), x11_intel);
        VERIFY_DATA(filesystem.get_inode_id_by_path("/etc/X11/Xorg.conf", 0), Xorg_conf);

        filesystem.remove_child_dentry_under_parent(X11, "Xorg.conf");

        try {
            filesystem.get_inode_id_by_path("/etc/X11/Xorg.conf", 0);
        } catch (HTMPFS_error_t & err) {
            VERIFY_DATA(err.my_errcode(), HTMPFS_NO_SUCH_FILE_OR_DIR);
        }

        try {
            filesystem.remove_child_dentry_under_parent(X11, "nvidia.conf");
        } catch (HTMPFS_error_t & err) {
            VERIFY_DATA(err.my_errcode(), HTMPFS_NO_SUCH_FILE_OR_DIR);
        }

        try {
            filesystem.remove_child_dentry_under_parent(FILESYSTEM_ROOT_INODE_NUMBER,
                                                        "etc");
        } catch (HTMPFS_error_t & err) {
            VERIFY_DATA(err.my_errcode(), HTMPFS_DIR_NOT_EMPTY);
        }
    }

    {
        /// instance 4: export filesystem map

        // create a new filesystem
        inode_smi_t filesystem(27);
        inode_id_t etc_id =
                filesystem.make_child_dentry_under_parent(FILESYSTEM_ROOT_INODE_NUMBER,
                                                          "etc", true);

        inode_id_t boot =
                filesystem.make_child_dentry_under_parent(FILESYSTEM_ROOT_INODE_NUMBER,
                                                          "boot", true);

        inode_id_t X11 =
                filesystem.make_child_dentry_under_parent(etc_id, "X11", true);
        inode_id_t x11_intel =
                filesystem.make_child_dentry_under_parent(X11, "x11_intel.conf", false);
        inode_id_t Xorg_conf =
                filesystem.make_child_dentry_under_parent(X11, "Xorg.conf", false);

        inode_id_t linux_kernel =
                filesystem.make_child_dentry_under_parent(boot, "linux", false);

        /* current filesystem layout:
         * /
         * |
         * |---- etc/
         * |        |------X11\
         * |                  |
         * |                  |------- x11_intel.conf
         * |                  |------- Xorg.conf
         * |
         * |---- boot\
         * |         |
         * |         |-------- linux
         * */

        filesystem.export_as_filesystem_map(0);
    }

    {
        /// mixed operation, random pathname + data

        std::map < std::string /* pathname */, std::string /* file content */ > files;
        std::vector < std::string > pathname_dictionary;

        auto generate_unique_name = [&]()->std::string
        {
            std::string name = gen_random_name(10);
            while (files.find(name) != files.end())
            {
                name = gen_random_name(128);
            }

            return name;
        };

        auto generate_random_num = [&](uint64_t start, uint64_t end)->uint64_t
        {
            std::random_device dev;
            std::string ret;
            std::mt19937 rng(dev());
            std::uniform_int_distribution<std::mt19937::result_type> dist255(start, end);
            return dist255(rng);
        };

        // ----------------------------------------------------------------------------------------- //

        const uint64_t filesystem_tree_size_seed = 8;
        inode_smi_t filesystem(27);

        for (uint64_t first_level_dir_count = 1;
             first_level_dir_count <= filesystem_tree_size_seed;
             first_level_dir_count++)
        {
            // create directory under root
            auto first_level_dir_name = generate_unique_name();
            filesystem.make_child_dentry_under_parent(FILESYSTEM_ROOT_INODE_NUMBER,
                                                      first_level_dir_name,
                                                      true);

            std::string subsequent_path_name = "/" + first_level_dir_name;

//            std::cout << "MKTAG\t" << subsequent_path_name << std::endl;
            pathname_dictionary.emplace_back(subsequent_path_name);

            for (uint64_t dir_count = 0; dir_count < first_level_dir_count; dir_count++)
            {
                // create subsequent directory
                auto target_name = generate_unique_name();

//                std::cout << "MKTAG\t" << subsequent_path_name << "/" << target_name << std::endl;
                pathname_dictionary.emplace_back(subsequent_path_name + "/" + target_name);

                auto parent = filesystem.get_inode_id_by_path(subsequent_path_name, 0);
                filesystem.make_child_dentry_under_parent(parent, target_name, true);

                // increase depth
                subsequent_path_name += "/" + target_name;
            }

            // write content
            std::string content = gen_random_data(generate_random_num(4096, 5133));
            auto * inode = filesystem.get_inode_by_id(
                    filesystem.get_inode_id_by_path(subsequent_path_name, 0)
            );
            inode->__override_dentry_flag(false);
            inode->write(content.c_str(), content.length(), 0);

            // update file map
            files.emplace(subsequent_path_name, content);
        }

        /// data verification

        // pathname verification, verify all existence of valid pathname
        // dictionary as base, verify filesystem
        for (const auto & i : pathname_dictionary)
        {
            filesystem.get_inode_id_by_path(i, 0);
        }

        // filesystem as base, verify dictionary
        auto vec = filesystem.export_as_filesystem_map(0);
        for (const auto & i : vec)
        {
            if (!can_find(pathname_dictionary, i))
            {
                return EXIT_FAILURE;
            }
        }

        // verify data
        for (const auto & i : files)
        {
            inode_id_t inode = filesystem.get_inode_id_by_path(i.first, 0);
            auto _data = filesystem.get_inode_by_id(inode)->to_string(0);
            if (!!memcmp(i.second.c_str(), _data.c_str(), MIN(i.second.length(), _data.length())))
            {
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}
