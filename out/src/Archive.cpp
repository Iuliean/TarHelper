#include <archive_entry.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <linux/limits.h>

#include "Archive.h"
#include "utils.h" 

std::shared_ptr<spdlog::logger> Archive::s_log = spdlog::stdout_color_mt("Archive");


//to reimplement
static int copy_data(struct archive *ar, struct archive *aw)
{
    int r;
    const void *buff;
    size_t size;
    int64_t offset;

    while(true){
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return (ARCHIVE_OK);
        if (r < ARCHIVE_OK)
            return (r);
        r = archive_write_data_block(aw, buff, size, offset);
        if (r < ARCHIVE_OK) 
            return (r);
    }
}

Archive::Archive(const std::string& path)
    :m_path(path)
{
    if(path == "" || path == "\0")
        throw archive_exception("Path is empty");
}

Archive::~Archive()
{
}

void Archive::list(std::vector<std::string>& buff)
{
    try
    {
        openArchive();   
        archive_entry* entry;
        while(archive_read_next_header(m_pArchiveDescrptor, &entry) == ARCHIVE_OK)
        {
            buff.push_back(archive_entry_pathname(entry));
            archive_read_data_skip(m_pArchiveDescrptor);
        }
        closeArchive();
    }
    catch(const std::exception& e)
    {
        throw;
    }
    
}

void Archive::decompress(const std::string& path, int decompressionOptions)
{
    try
    {

        archive_entry* entry;
        archive* out;
        char workingDir[PATH_MAX];
        openArchive();

        if(getcwd(workingDir, PATH_MAX) == NULL)
        {
            throw archive_exception(std::string("Failed to get current directory: {}") + std::to_string(errno));
        }

        if(mkdir(path.c_str(), 0777) == 0 && errno != EEXIST && errno != 0)
        {
            std::string msg = "Failed to make directory " + path + " : " + std::to_string(errno);
            throw archive_exception(std::move(msg));
        }

        if(chdir(path.c_str()) != 0)
        {
            std::string msg = "Failed to change directory to " + path + " : " + std::to_string(errno);
            throw archive_exception(std::move(msg));
        }

        out = archive_write_disk_new();
        archive_write_disk_set_options(out, decompressionOptions);
        archive_write_disk_set_standard_lookup(out);

        int result;
        //abstract this to another method that handels it on a per entry case;
        while(true)
        {
            result = archive_read_next_header(m_pArchiveDescrptor, &entry);
            if(result == ARCHIVE_EOF)
                break;
            switch(result)
            {
                case ARCHIVE_RETRY: continue;
                case ARCHIVE_WARN:
                {
                    s_log->warn("Failed to read entry header: {}", archive_error_string(m_pArchiveDescrptor));
                }break;
                case ARCHIVE_FAILED:
                case ARCHIVE_FATAL:
                {
                    s_log->error("Fatal error reading entry header: {}", archive_error_string(m_pArchiveDescrptor));
                    exit(1);
                }break;
            }
            result = archive_write_header(out, entry);

            switch(result)
            {
                case ARCHIVE_OK:
                {
                    if(archive_entry_size(entry) > 0)
                        copy_data(m_pArchiveDescrptor, out);
                }break;
                case ARCHIVE_RETRY:
                {
                    result = archive_write_header(out, entry);
                    if(result == ARCHIVE_OK)
                        break;
                    if(result >= ARCHIVE_WARN)
                    {
                        s_log->warn("{}", archive_error_string(out));
                        archive_write_close(out);
                        archive_write_free(out);
                        closeArchive();
                        return;
                    }
                    else
                    {
                        s_log->error("{}", archive_error_string(out));
                        exit(1);    
                    }   
                }break;
                case ARCHIVE_WARN:
                {
                    s_log->warn("Failed to read entry header: {}", archive_error_string(out));
                }break;
                case ARCHIVE_FAILED:
                case ARCHIVE_FATAL:
                {
                    s_log->error("Fatal error reading entry header: {}", archive_error_string(out));
                    exit(1);
                }break;
            }
        }

        archive_write_close(out);
        archive_write_free(out);

        if(chdir(workingDir) != 0)
        {
            std::string msg = "Failed to change directory to " + path + " : " + std::to_string(errno);
            throw archive_exception(std::move(msg));
        }

        closeArchive();
    }
    catch(const std::exception& e)
    {
        throw;
    }
    
}

//private

void Archive::openArchive()
{

    m_pArchiveDescrptor = archive_read_new();
    archive_read_support_filter_all(m_pArchiveDescrptor);
    archive_read_support_format_all(m_pArchiveDescrptor);
    m_pFd = fopen(m_path.c_str(), "r");
    if (m_pFd == NULL)
    {
        throw archive_exception("Cannot open file " + m_path + "reason: " + std::to_string(errno));
    }

    if(archive_read_open_FILE(m_pArchiveDescrptor, m_pFd) != ARCHIVE_OK)
    {
        std::string msg(archive_error_string(m_pArchiveDescrptor));
        throw archive_exception(msg);
    }
}

void Archive::closeArchive()
{
    fclose(m_pFd);
    m_pFd = nullptr;
    archive_read_free(m_pArchiveDescrptor);
}