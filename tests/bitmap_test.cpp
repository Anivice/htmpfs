/** @file
 *
 * This file handles test for bitmap
 */

#include <htmpfs/bitmap_t.h>
#include <htmpfs/htmpfs.h>
#include <iostream>
#include <debug.h>
#include <random>
#include <vector>
#include <chrono>
#include <algorithm>

#define VERIFY_DATA(val, tag) if ((tag) != (val)) { return EXIT_FAILURE; } __asm__("nop")

std::vector < uint64_t > generate_unique_num(uint64_t start, uint64_t end, uint64_t count)
{
    std::vector < uint64_t > numbers;
    std::vector < uint64_t > ret;
    for (uint64_t i = start; i < end; i++) {
        numbers.push_back(i);
    }

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(numbers.begin(), numbers.end(), std::default_random_engine(seed));

    for (int i = 0; i < count; i++)
    {
        ret.push_back(numbers[i]);
    }

    return ret;
}

uint64_t generate_random_num (uint64_t start, uint64_t end)
{
    std::random_device dev;
    std::string ret;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist255(start, end);
    return dist255(rng);
};

int main()
{
    {
        /// instance 1: bit operation, set_bit_of

        INSTANCE("instance 1: bit operation, set_bit_of");
        uint8_t bit = 0x00;
        set_bit_of(bit, 0);
        VERIFY_DATA(bit, 0x01);
    }

    {
        /// instance 2: bit operation, remove_bit_of

        INSTANCE("instance 2: bit operation, remove_bit_of");
        uint8_t bit = 0x00;
        set_bit_of(bit, 1);
        set_bit_of(bit, 0);
        remove_bit_of(bit, 0);
        VERIFY_DATA(bit, 0x02);
    }

    {
        /// instance 3: bit operation, get_bit_of

        INSTANCE("instance 3: bit operation, get_bit_of");
        uint8_t bit = 0x00;
        set_bit_of(bit, 1);
        set_bit_of(bit, 0);
        remove_bit_of(bit, 0);
        VERIFY_DATA(get_bit_of(bit, 1), true);
        VERIFY_DATA(bit, 0x02);
    }

    {
        /// instance 4, mixed bitmap operation

        INSTANCE("instance 4, mixed bitmap operation");

//        for (uint64_t count = 0; count < 30; count++)
        {
            inode_smi_t filesystem(4);
            for (uint64_t i = 0; i < 2; i++)
            {
                auto id = filesystem.request_buffer_allocation();
                filesystem.get_buffer_by_id(id.id)->truncate(4);
            }

            bitmap_t bitmap(&filesystem, 0, 64);

            uint64_t init_count = generate_random_num(32, 64);
            uint64_t ops_count = 0;
            auto ops_map =
                    generate_unique_num(0,
                                        63,
                                        init_count);

            // randomly set bits in bitmap
            for (auto &i: ops_map) {
                bitmap.set_bit_in_map(i, 1);
            }

            // count how many bits are set in bitmap
            for (uint64_t i = 0; i < 64; i++)
            {
                if (bitmap.get_bit_in_map(i)) {
                    ops_count++;
                }
            }

            if (ops_count != init_count) {
                return EXIT_FAILURE;
            }
        }

        return EXIT_SUCCESS;
    }
}
