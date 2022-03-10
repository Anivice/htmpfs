#include <ll_io/ll_io.h>
#include <fstream>
#include <iostream>

int main()
{
    std::fstream file("tmp", std::ios::out);
    file.close();

    ll_io dev;
    dev.open("tmp");
    dev.write("FFF", 3, 1);
    char buff[32] {};
    dev.read(buff, 32, 1);
    std::cout << buff << std::endl;
}
