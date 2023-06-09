#include <archive.h>
#include <archive_entry.h>
#include <exception>
#include <fcntl.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <dirent.h>
#include <stack>
#include <type_traits>


#include "TarMaker.h"
#include "utils.h"

#define GET_ERRNO_STR() std::string(strerror(errno))

std::shared_ptr<spdlog::logger> TarMaker::s_log = spdlog::stdout_color_mt("TarMaker");

static const CompressionAlgorithm::Func ALGORITHM_IMPLEMENTATION[] ={
    archive_write_add_filter_gzip,
    archive_write_add_filter_bzip2,
    archive_write_add_filter_lz4,
    archive_write_add_filter_lzma,
    archive_write_add_filter_lzip,
    archive_write_add_filter_xz,
    archive_write_add_filter_uuencode,
};

static std::string readsymlink(const std::string& path)
{
	char buf[PATH_MAX];
	ssize_t len;
	if ((len = readlink(path.c_str(), buf, sizeof(buf)-1)) != -1) {
		buf[len] = '\0';
		return std::string(buf);
	}
	return "";
}

CompressionAlgorithm::CompressionAlgorithm(CompressionType type)
    : m_algorithm(ALGORITHM_IMPLEMENTATION[type])
{
}

CompressionAlgorithm::~CompressionAlgorithm()
{
}

TarMaker::TarMaker(const std::string& path)
    : TarMaker(path, CompressionType::GZIP, TarOptions::NONE)
{
}

TarMaker::TarMaker(const std::string& path, CompressionType alg, int options)
    : m_path(path), m_algorithm(alg), m_options((TarOptions)options)
{
    try 
    {
        openArchive();
    } 
    catch (std::exception& e)
    {
        throw;
    }
}

TarMaker::~TarMaker()
{
}

void TarMaker::addFile(const std::string& path, const std::string& pathInArchive)
{
    struct stat attributes;
    getFileAttibutes(path, &attributes);

    archive_entry* entry = archive_entry_new();
    archive_entry_set_pathname(entry, pathInArchive.c_str());

    if(isSymlink(path))
    {
        std::string link = readsymlink(path);
        archive_entry_set_filetype(entry, AE_IFLNK);
        archive_entry_set_symlink(entry, link.c_str());
        archive_write_header(m_pArchiveDescrptor, entry);
        return;
    }

    archive_entry_copy_stat(entry, &attributes);
    if(!(m_options & TarOptions::PreserveFilePermissions))
    {
        if(attributes.st_mode & S_IXUSR)
            archive_entry_set_perm(entry, 0777);
        else
            archive_entry_set_perm(entry, 0666);
    }
    archive_write_header(m_pArchiveDescrptor, entry);
    try
    {
        writeFileToArchive(path);
    }
    catch(const std::exception& e)
    {
        throw;
    }

    archive_entry_free(entry);
}

bool TarMaker::addDirectory(std::string path, const std::string& pathInArchive, size_t depth)
{
    if(!isDir(path))
        return false;

    std::stack<DIR*> toTraverse;

    DIR* dir = opendir(path.c_str());
    if(dir == nullptr)
        throw archive_exception("Could not open: " + GET_ERRNO_STR());

    dirent* entry = nullptr;

    std::string archiveLocation = pathInArchive;
    std::string diskLocation    = path;

    if(pathInArchive.back() != '/' && pathInArchive != "")
        archiveLocation.push_back('/');
    if(path.back() != '/')
    {
        diskLocation.push_back('/');
        path.push_back('/');
    }
    
    try
    {
        while(true)
        {
            entry = readdir(dir);
            //if reads null means either error or nothing left to read
            if(!entry)
            {
                if (errno != 0)
                    throw archive_exception("Could not read: " + GET_ERRNO_STR());
                else
                {
                    //if toTraverse is empty there are no other directories to traverse
                    if (toTraverse.empty())
                        break;
                    //else done with current folder close it and move to the next
                    else
                    {
                        closedir(dir);
                        dir = toTraverse.top();
                        toTraverse.pop();
                        diskLocation.pop_back();
                        
                        size_t previousPath = diskLocation.rfind("/") + 1;
                        diskLocation.erase(diskLocation.begin() + previousPath, diskLocation.end());
                        depth++;
                        continue;
                    }
                }
            }

            std::string filename = entry->d_name;
            if(filename == ".")
            {
                archive_entry* entry = archive_entry_new();
                std::string destination = archiveLocation;
                destination.append(
                    diskLocation.substr(path.length())
                );
                struct stat directoryAttributes;
                stat(diskLocation.c_str(), &directoryAttributes);
                archive_entry_copy_stat(entry, &directoryAttributes);
                archive_entry_set_pathname(entry, destination.c_str());
                archive_entry_set_filetype(entry, AE_IFDIR);
                if(!(m_options & TarOptions::PreserveDirectoryPermissions))
                    archive_entry_set_perm(entry, 0755);
                archive_write_header(m_pArchiveDescrptor, entry);
                archive_entry_free(entry);
                continue;
            }
            if(filename == "..")
            {
                continue;
            }

            if(isDir(diskLocation + filename))
            {
                //if maximum depth is reached do not traverse next directory;
                if(!depth)
                    continue;
                
                diskLocation.append(filename);
                diskLocation.push_back('/');
                toTraverse.push(dir);
                dir = opendir(diskLocation.c_str());
                
                if(dir == nullptr && errno == EACCES)
                {
                    closedir(dir);
                    dir = toTraverse.top();
                    toTraverse.pop();
                    throw archive_exception("Could not open dir: " + GET_ERRNO_STR());
                }
                else
                depth--;
            }
            else
            {
                std::string destination = archiveLocation;
                destination.append(
                    diskLocation.substr(path.length())
                );
                destination.append(filename);
                s_log->info("Archiving file: {} to archive location: {}", diskLocation + filename, destination);
                addFile(diskLocation + filename, destination);
            }
        }
    }          
    catch(const std::exception& e)
    {
        closedir(dir);
        while(!toTraverse.empty())
        {
            closedir(toTraverse.top());
            toTraverse.pop();
        }
        throw;
    }

    closedir(dir);
    return true;
}

void TarMaker::openArchive()
{
    m_pArchiveDescrptor = archive_write_new();
    m_algorithm(m_pArchiveDescrptor);
    archive_write_set_format_ustar(m_pArchiveDescrptor); 
    
    m_pFd = fopen(m_path.c_str(), "w");
    if (m_pFd == NULL)
    {
        throw archive_exception("Cannot open file " + m_path + " reason: " + GET_ERRNO_STR());
    }

    if(archive_write_open_FILE(m_pArchiveDescrptor, m_pFd) != ARCHIVE_OK)
    {
        std::string msg(archive_error_string(m_pArchiveDescrptor));
        throw archive_exception(msg);
    }
}

void TarMaker::closeArchive()
{
    archive_write_close(m_pArchiveDescrptor);
    archive_write_free(m_pArchiveDescrptor);
    fclose(m_pFd);
}

//private
void TarMaker::writeFileToArchive(const std::string& path)
{
    static const size_t BUFF_SIZE = 1024;
    char buffer[BUFF_SIZE] = {};

    FILE* file = fopen(path.c_str(), "r");
    if (file == NULL)
    {
        throw archive_exception("Cannot open file " + path + " reason: " + GET_ERRNO_STR());
    }

    while(true)
    {
        size_t read = fread(buffer, sizeof(*buffer), BUFF_SIZE, file);
        
        if(read > 0)
            archive_write_data(m_pArchiveDescrptor, buffer, read);
        if(feof(file))
            break;
        if(ferror(file))
        {
            fclose(file);
            throw archive_exception("Failed to read from file " + path + " : " + GET_ERRNO_STR());
        }
    }
    fclose(file);
}

void TarMaker::getFileAttibutes(const std::string& path, struct stat* attributes)
{
    if(m_options & TarOptions::DereferenceSymlink)
        stat(path.c_str(), attributes);
    else
        lstat(path.c_str(), attributes);
}

bool TarMaker::checkFileTypeAttribute(const std::string& path, int attribute)
{
    struct stat attributes;
    if(m_options & TarOptions::DereferenceSymlink)
        stat(path.c_str(), &attributes);
    else
        lstat(path.c_str(), &attributes);
    return (attributes.st_mode & S_IFMT) == attribute;
}