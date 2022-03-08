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

void show_vector(const std::vector <std::string> & vec)
{
    for (const auto & i : vec)
    {
        std::cout << i << std::endl;
    }
}

bool compare_two_vec(const std::vector < std::string > & _vec_1,
                     const std::vector < std::string > & _vec_2)
{
    if (_vec_1.size() != _vec_2.size())
    {
        return false;
    }

    for (const auto& i : _vec_2)
    {
        if (std::find(_vec_1.begin(), _vec_1.end(), i) == _vec_1.end())
        {
            return false;
        }
    }

    return true;
}

int main()
{
    {
        /// instance 1: filesystem snapshot, no content change

        INSTANCE("FILESYSTEM: instance 1: filesystem snapshot, no content change");
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

        auto snapshot_id = filesystem.create_snapshot_volume();

        filesystem.remove_child_dentry_under_parent(
                FILESYSTEM_ROOT_INODE_NUMBER,
                "linux.boot");

        show_vector(filesystem.export_as_filesystem_map(0));
        show_vector(filesystem.export_as_filesystem_map(snapshot_id));
    }

    {
        /// instance 2: filesystem snapshot, delete snapshot volume

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

        auto snapshot_id = filesystem.create_snapshot_volume();

        filesystem.remove_child_dentry_under_parent(
                FILESYSTEM_ROOT_INODE_NUMBER,
                "linux.boot");

        show_vector(filesystem.export_as_filesystem_map(0));
        show_vector(filesystem.export_as_filesystem_map(snapshot_id));

        filesystem.delete_snapshot_volume(snapshot_id);

        try {
            filesystem.get_inode_id_by_path(make_path_with_version("/etc", snapshot_id));
        } catch (HTMPFS_error_t & err) {
            if (err.my_errcode() != HTMPFS_NO_SUCH_SNAPSHOT)
            {
                return EXIT_FAILURE;
            }
        }
    }

    {
        /// mixed operation, verify file content and pathname

        INSTANCE("FILESYSTEM: mixed operation, verify file content and pathname");
        /// generate random filesystem tree
        //////////////////////////////////////////////////////////////////////////////

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

        const uint64_t filesystem_tree_size_seed = 16;
        inode_smi_t filesystem(4);

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

                auto parent = filesystem.get_inode_id_by_path(make_path_with_version(subsequent_path_name, 0));
                filesystem.make_child_dentry_under_parent(parent, target_name, true);

                // increase depth
                subsequent_path_name += "/" + target_name;
            }

            // write content
            std::string content = gen_random_data(generate_random_num(4096, 5133));
            auto * inode = filesystem.get_inode_by_id(
                    filesystem.get_inode_id_by_path(
                            make_path_with_version(subsequent_path_name, 0))
            );
            inode->__override_dentry_flag(false);
            inode->write(content.c_str(), content.length(), 0);

            // update file map
            files.emplace(subsequent_path_name, content);
        }

        //////////////////////////////////////////////////////////////////////////////

        // create a snapshot of current filesystem
        filesystem.create_snapshot_volume();

        show_vector(filesystem.export_as_filesystem_map(1));

        /// verify snapshot tree

        // pathname verification, verify all existence of valid pathname
        // dictionary as base, verify filesystem
        for (const auto & i : pathname_dictionary)
        {
            filesystem.get_inode_id_by_path(make_path_with_version(i, 1));
        }

        // filesystem as base, verify dictionary
        auto vec = filesystem.export_as_filesystem_map(1);
        for (const auto & i : vec)
        {
            if (!can_find(pathname_dictionary, i))
            {
                return EXIT_FAILURE;
            }
        }

        /// randomly modify filesystem tree

        // sort pathname by length, so the deepest inode will be deleted first
        std::sort(pathname_dictionary.begin(), pathname_dictionary.end(), []
                (const std::string& first, const std::string& second){
            return first.size() > second.size();
        });

        uint64_t random_deletion_num = generate_random_num(filesystem_tree_size_seed,
                                                           pathname_dictionary.size() / 2);
        for (uint64_t i = 0; i < random_deletion_num; i++)
        {
            auto it = files.find(*pathname_dictionary.begin());
            if (it != files.end()) {
                files.erase(it);
            }

            filesystem.remove_inode_by_path(*pathname_dictionary.begin());
            pathname_dictionary.erase(pathname_dictionary.begin());
        }

        // yea let me take a snapshot real quick
        filesystem.create_snapshot_volume();

        if (!compare_two_vec(
                filesystem.export_as_filesystem_map(0),
                filesystem.export_as_filesystem_map(2)))
        {
            return EXIT_FAILURE;
        }

        auto _vec = filesystem.export_as_filesystem_map(2);
        std::sort(_vec.begin(), _vec.end(), []
                (const std::string& first, const std::string& second){
            return first.size() > second.size();
        });

        if (!compare_two_vec(_vec, pathname_dictionary))
        {
            return EXIT_FAILURE;
        }

        /// verify content in data

        // verify data
        for (const auto & i : files)
        {
            inode_id_t inode = filesystem.get_inode_id_by_path(make_path_with_version(i.first, 1));
            auto _data = filesystem.get_inode_by_id(inode)->to_string(1);
            if (!!memcmp(i.second.c_str(), _data.c_str(), MIN(i.second.length(), _data.length())))
            {
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}
