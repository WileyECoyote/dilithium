
#ifndef GLOBALS_H
#define GLOBALS_H

extern bool  BeQuite;
extern bool  DebugMode;
extern bool  DaemonMode;
extern bool  DropPrivileges;
extern bool  KillX;
extern bool  UseXinit;
extern bool  Verbose;
extern bool  DilithiumLog;

#include <list>
#include <string>
#include <exception>

/******************************************************************************/

typedef enum {
    ERROR_LOCK_ERROR,
    ERROR_FILE_ERROR,
    ERROR_INVALID_ADDRESS,
    ERROR_INVALID_COOKIE,
    ERROR_COND_ERROR,
    ERROR_UNIMPLEMENTED
} ErrorType;

class Error : public std::exception
{
public:
    const ErrorType type;
    const std::string message;

    Error(ErrorType type_, std::string message_)
        : type(type_), message(message_) {}

    virtual ~Error() throw() {}

    virtual const char* what() const throw()
        { return message.c_str(); }
};

/******************************************************************************/
#endif