#include <debug.h>
#include <iostream>

/** @file
 *
 * This file handles test for runtime debug support
 */

void func2()
{
    FUNCTION_INFO(2);
    OBTAIN_STACK_FRAME(2);
}

void func1()
{
    FUNCTION_INFO(2);
    OBTAIN_STACK_FRAME(2);

    func2();
}

int main()
{
    FUNCTION_INFO(2);
    OBTAIN_STACK_FRAME(2);

    func1();

    return EXIT_SUCCESS;
}
