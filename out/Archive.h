#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <archive.h>
#include <spdlog/spdlog.h>

class Archive
{
public:
    Archive(const std::string& path);
    ~Archive();
    void list(std::vector<std::string>& buff);
    void decompress(const std::string& path,
                        int decompressionOptions =  DecompressionOptions::PRESERVE_TIME |
                                                    DecompressionOptions::PRESERVE_FFLAGS|
                                                    DecompressionOptions::PRESERVE_PERSMISSIONS|
                                                    DecompressionOptions::PRESERVE_ACL);
public:
    enum DecompressionOptions
    {
        PRESERVE_OWNER          = ARCHIVE_EXTRACT_OWNER,
        PRESERVE_PERSMISSIONS   = ARCHIVE_EXTRACT_PERM,
        PRESERVE_TIME           = ARCHIVE_EXTRACT_TIME,
        NO_OVERWRITE            = ARCHIVE_EXTRACT_NO_OVERWRITE,
        EXTRACT_UNLINK          = ARCHIVE_EXTRACT_UNLINK,
        PRESERVE_ACL            = ARCHIVE_EXTRACT_ACL,    
        PRESERVE_FFLAGS         = ARCHIVE_EXTRACT_FFLAGS,
        PRESERVE_XATTR          = ARCHIVE_EXTRACT_XATTR,
        SECURE_SYMLINKS         = ARCHIVE_EXTRACT_SECURE_SYMLINKS,
        SECURE_NODOTDOT         = ARCHIVE_EXTRACT_SECURE_NODOTDOT,
        NO_PARENT               = ARCHIVE_EXTRACT_NO_AUTODIR,
        NO_OVERWRITE_IF_NEWER   = ARCHIVE_EXTRACT_NO_OVERWRITE_NEWER,
        SPARSE                  = ARCHIVE_EXTRACT_SPARSE,
        MAC_METADATA            = ARCHIVE_EXTRACT_MAC_METADATA,
        NO_HFS_COMPRESSION      = ARCHIVE_EXTRACT_NO_HFS_COMPRESSION,
        FORCE_HFS_COMPRESSION   = ARCHIVE_EXTRACT_HFS_COMPRESSION_FORCED,
        NO_ABSOLUTEPATHS        = ARCHIVE_EXTRACT_SECURE_NOABSOLUTEPATHS,
        CLEAR_NOCHANGE_FFLAGS   = ARCHIVE_EXTRACT_CLEAR_NOCHANGE_FFLAGS,
        SAFE_WRITES             = ARCHIVE_EXTRACT_SAFE_WRITES, 
    };

private:
    void openArchive();
    void closeArchive();
private:
    std::string m_path;
    FILE* m_pFd;
    archive* m_pArchiveDescrptor;
    static std::shared_ptr<spdlog::logger> s_log;
};
#endif //ARCHIVE_H