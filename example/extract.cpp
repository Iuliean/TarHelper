#include <Archive.h>
#include <iostream>

int main(int argc, char** argv)
{
    Archive arch("src.tar.gz");
    arch.decompress("out/");

    return 0;
}