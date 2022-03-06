#include <htmpfs.h>
#include <iostream>
#include <string>
#include <random>
#include <map>
#include <cmath>

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
    static const char alphanum[] =
            "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(length);

    std::random_device dev;
    std::string ret;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist255(0, sizeof(alphanum) - 1);

    for (int i = 0; i < length; ++i)
    {
        tmp_s += alphanum[dist255(rng)];
    }

    return tmp_s;
}

class simple_inode
{
public:
    std::string data;
    std::string pathname;

    std::vector < simple_inode > children;
};

int main()
{
    // generate a huge amount of random data
    const std::string data = gen_random_data(5391);

    {
        /// instance 1: filesystem bare I/O capability

        // create a new filesystem
        inode_smi_t filesystem(27);

        // write data
        filesystem.get_inode_by_id(FILESYSTEM_ROOT_INODE_NUMBER)
                  ->write(data.c_str(), data.length(), 0);

        VERIFY_DATA(filesystem.get_inode_by_id(FILESYSTEM_ROOT_INODE_NUMBER)
                              ->to_string(0),
                    data);
    }

    {
        /// instance 2: create a sub directory, write data, both successful and failed

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

        filesystem.get_inode_by_id(linux_boot)->write(data.c_str(), data.length(), 0);
        filesystem.get_inode_by_id(Xorg_conf)->write(data.c_str(), data.length(), 0);

        try
        {
            filesystem.make_child_dentry_under_parent(Xorg_conf, "nvidia.conf");
        }
        catch (HTMPFS_error_t & err)
        {
            VERIFY_DATA(err.my_errcode(), HTMPFS_NOT_A_DIRECTORY);
        }

        VERIFY_DATA(filesystem.get_inode_by_id(linux_boot)->to_string(0), data);
        VERIFY_DATA(filesystem.get_inode_by_id(Xorg_conf)->to_string(0), data);
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
        /// mixed operation, random pathname + data

        std::map < std::string, std::string > files;

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

        inode_smi_t filesystem(27);

        for (uint64_t first_level_dir_count = 1; first_level_dir_count <= 32; first_level_dir_count++)
        {
            // create directory under root
            auto first_level_dir_name = generate_unique_name();
            filesystem.make_child_dentry_under_parent(FILESYSTEM_ROOT_INODE_NUMBER,
                                                      first_level_dir_name,
                                                      true);

            std::string subsequent_path_name = "/" + first_level_dir_name;
            std::string shared_target_name;
            for (uint64_t dir_count = 0; dir_count < first_level_dir_count; dir_count++)
            {
                // create subsequent directory
                auto target_name = generate_unique_name();
                std::cout << "MKTAG\t" << subsequent_path_name << "/" << target_name << std::endl;
                auto parent = filesystem.get_inode_id_by_path(subsequent_path_name, 0);
                filesystem.make_child_dentry_under_parent(parent, target_name, true);

                // increase depth
                shared_target_name = target_name;
                subsequent_path_name += "/" + target_name;
            }

            // write content
            std::string content = gen_random_data(generate_random_num(4096, 5133));
            filesystem.get_inode_by_id(filesystem.get_inode_id_by_path(subsequent_path_name, 0))->
                write(content.c_str(), content.length(), 0);

            // update file map
            files.emplace(shared_target_name, content);
        }

        /// data verification


    }

    return EXIT_SUCCESS;
}
