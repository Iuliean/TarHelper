#ifndef TAR_MAKER_H
#define TAR_MAKER_H
#include <fcntl.h>
#include <string>
#include <spdlog/spdlog.h>
#include <archive.h>
#include <sys/stat.h>


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

enum TarOptions: int
{
    NONE= 0b0,
    DereferenceSymlink =0b01,
    PreservePermissions=0b10
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
    TarMaker() = delete;
    TarMaker(const std::string& path);
    TarMaker(const std::string& path, CompressionType alg, int options);
    ~TarMaker();

    void addFile(const std::string& path);
    void addFile(const std::string& path, const std::string& pathInArchive);

    bool addDirectory(const std::string& path, size_t depth = SIZE_MAX);
    bool addDirectory(const std::string&, const std::string& pathInArchive, size_t depth = SIZE_MAX);

    void openArchive();
    void closeArchive();
private:
    void writeFileToArchive(const std::string& path);
    bool isDir(const std::string& path);
    bool isSymlink(const std::string& path);
    void getFileAttibutes(const std::string& , struct stat* attributes);
    bool checkFileTypeAttribute(const std::string& path, int attribute);

private:
    FILE* m_pFd;
    std::string m_path;
    CompressionAlgorithm m_algorithm;
    TarOptions m_options;
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

inline bool TarMaker::isDir(const std::string& path)
{
    return checkFileTypeAttribute(path, S_IFDIR);
}

inline bool TarMaker::isSymlink(const std::string& path)
{
    return checkFileTypeAttribute(path, S_IFLNK);
}

inline int CompressionAlgorithm::operator()(struct archive* arch)
{
    return m_algorithm(arch);
}
#endif //TAR_MAKER_H