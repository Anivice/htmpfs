#include <buffer_t.h>
#include <string>
#include <cstring>

/** @file
 *
 * This file handles test for buffer_t
 */

#define VERIFY_DATA_OPS_LEN(operation, len) if ((operation) != len) { return EXIT_FAILURE; } __asm__("nop")

int main(int argc, char ** argv)
{
    {
        /// instance 1: default constructor
        buffer_t buffer;
        if (!buffer.empty())
        {
            return EXIT_FAILURE;
        }
    }

    {
        /// instance 2: constructing with data
        const char * hello_world = "Hello, world!";
        buffer_t buffer(hello_world, strlen(hello_world));
        if (buffer.to_string() != hello_world)
        {
            return EXIT_FAILURE;
        }
    }

    {
        /// instance 3: write to an empty buffer, append enabled
        const char * hello_world = "Hello, world!";
        buffer_t buffer;

        VERIFY_DATA_OPS_LEN(buffer.write(hello_world, strlen(hello_world), 0),
                            strlen(hello_world));

        if (buffer.to_string() != hello_world)
        {
            return EXIT_FAILURE;
        }
    }

    {
        /// instance 4: write to a filled buffer, append disabled
        const char * hello_world =      "Hello, world!";
        const char * Kello_world =      "Kello, world!MPKS";
        const char * kello_world_norm = "Kello, world!";
        buffer_t buffer;

        VERIFY_DATA_OPS_LEN(buffer.write(hello_world, strlen(hello_world), 0),
                            strlen(hello_world));

        VERIFY_DATA_OPS_LEN(buffer.write(Kello_world, strlen(Kello_world), 0, false),
                            strlen(hello_world));

        if (buffer.to_string() != kello_world_norm)
        {
            return EXIT_FAILURE;
        }
    }

    {
        /// instance 5: write to a filled buffer, with offset
        const char * hello_world =      "Hello, world!";
        const char * hKllo_world =      "K";
        const char * hKllo_world_norm = "HKllo, world!";
        buffer_t buffer;

        VERIFY_DATA_OPS_LEN(buffer.write(hello_world, strlen(hello_world), 0),
                            strlen(hello_world));

        VERIFY_DATA_OPS_LEN(buffer.write(hKllo_world, 1, 1), 1);
        if (buffer.to_string() != hKllo_world_norm)
        {
            return EXIT_FAILURE;
        }
    }

    {
        /// instance 6: read from buffer, non-exceeded read
        const char * hello_world = "Hello, world!";
        char cbuffer[32]{};
        buffer_t buffer(hello_world, strlen(hello_world));

        VERIFY_DATA_OPS_LEN(buffer.read(cbuffer, strlen(hello_world), 0),
                            strlen(hello_world));

        if (buffer.to_string() != cbuffer)
        {
            return EXIT_FAILURE;
        }
    }

    {
        /// instance 7: read from buffer, exceeded read
        const char * hello_world = "Hello, world!";
        char cbuffer[32]{};
        buffer_t buffer(hello_world, strlen(hello_world));

        VERIFY_DATA_OPS_LEN(buffer.read(cbuffer, sizeof(cbuffer), 0),
                            strlen(hello_world));

        if (buffer.to_string() != cbuffer)
        {
            return EXIT_FAILURE;
        }
    }

    {
        /// instance 8: read/write(append disabled) offset > data bank size
        buffer_t buffer;
        VERIFY_DATA_OPS_LEN(buffer.read(nullptr, 1, 1), 0);
        VERIFY_DATA_OPS_LEN(buffer.write(nullptr, 1, 1, false), 0);
        if (!buffer.empty())
        {
            return EXIT_FAILURE;
        }
    }

    {
        /// instance 9: normal write, append disabled
        const char * hello_world = "Hello, world!";
        buffer_t buffer(hello_world, strlen(hello_world));
        VERIFY_DATA_OPS_LEN(buffer.write("123", 3, 0, false), 3);
        if (buffer.to_string() != "123lo, world!")
        {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
