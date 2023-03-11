#ifndef UTILS_H
#define UTILS_H
#include <stdexcept>

class archive_exception : public std::runtime_error
{
public:
    archive_exception(const std::string& reason);
    virtual ~archive_exception();
    virtual const char* what()const noexcept override;
};

#endif //UTILS_H