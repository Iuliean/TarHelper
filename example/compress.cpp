#include <TarMaker.h>

int main(int argc, char** argv)
{
    TarMaker tar("src.tar.gz", CompressionType::GZIP);
    tar.openArchive();
    tar.addDirectory("src/", "src");
    tar.addFile("include/Archive.h", "Archive.h");
    tar.closeArchive();
    return 0;
}