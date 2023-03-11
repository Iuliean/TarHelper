#include "utils.h"

//archive_exception
archive_exception::archive_exception(const std::string& reason)
    : std::runtime_error(reason)
{   
}

archive_exception::~archive_exception()
{

}
