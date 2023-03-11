#ifndef TAR_MAKER_H
#define TAR_MAKER_H
#include <string>
#include <spdlog/spdlog.h>
#include <archive.h>


enum CompressionType : char
{
    GZIP    = 0,
    BZIP    = 1,
    LZ4     = 2,
    LZMA    = 3,
    LZIP    = 4,
    LZOP    = 4,
    XZ      = 5,
    UU      = 6,
};

class CompressionAlgorithm
{
public:
    using Func = int(*)(struct archive*);
    CompressionAlgorithm(CompressionType type);
    ~CompressionAlgorithm();
    int operator()(struct archive* arch);
private:
    Func m_algorithm;
};

class TarMaker
{
public:
    TarMaker(const std::string& path, CompressionType alg);
    ~TarMaker();

    void addFile(const std::string& path);
    void addFile(const std::string& path, const std::string& pathInArchive);

    bool addDirectory(const std::string& path, size_t depth = SIZE_MAX);
    bool addDirectory(const std::string&, const std::string& pathInArchive, size_t depth = SIZE_MAX);

    void openArchive();
    void closeArchive();
private:
    void writeFileToArchive(const std::string& path);
private:
    FILE* m_pFd;
    std::string m_path;
    CompressionAlgorithm m_algorithm;
    archive* m_pArchiveDescrptor;
    static std::shared_ptr<spdlog::logger> s_log;
};


inline void TarMaker::addFile(const std::string& path)
{
    try
    {
        addFile(path, path);
    } 
    catch(std::exception& e)
    {
        throw;
    }
}

inline bool TarMaker::addDirectory(const std::string& path, size_t depth)
{
    try
    {
        return addDirectory(path, path, depth);
    } 
    catch(const std::exception& e)
    {
        throw;
    }
}

inline int CompressionAlgorithm::operator()(struct archive* arch)
{
    return m_algorithm(arch);
}
#endif //TAR_MAKER_H