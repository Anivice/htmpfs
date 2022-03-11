/** @file
 *
 * This file handles test for bitmap
 */

#include <htmpfs/bitmap_t.h>
#include <iostream>
#include <debug.h>

#define VERIFY_DATA(val, tag) if ((tag) != (val)) { return EXIT_FAILURE; } __asm__("nop")

int main()
{
    {
        /// instance 1: bit operation, set_bit_of

        INSTANCE("instance 1: bit operation, set_bit_of");
        uint64_t bit = 0x00;
        set_bit_of(bit, 0);
        VERIFY_DATA(bit, 0x01);
    }

    {
        /// instance 2: bit operation, remove_bit_of

        INSTANCE("instance 2: bit operation, remove_bit_of");
        uint64_t bit = 0x00;
        set_bit_of(bit, 1);
        set_bit_of(bit, 0);
        remove_bit_of(bit, 0);
        VERIFY_DATA(bit, 0x02);
    }

    {
        /// instance 3: bit operation, get_bit_of

        INSTANCE("instance 3: bit operation, get_bit_of");
        uint64_t bit = 0x00;
        set_bit_of(bit, 1);
        set_bit_of(bit, 0);
        remove_bit_of(bit, 0);
        VERIFY_DATA(get_bit_of(bit, 1), true);
        VERIFY_DATA(bit, 0x02);
    }
}
