#pragma once
#include <Typedefs/Typedefs.h>
#include <stdarg.h>

struct ILogger
{
    void* user_data;
    ActionParams<ILogger*, const char*> log;
    ActionParams<ILogger*, const char*> info;
    ActionParams<ILogger* , size_t> new_line;
    ActionParams<ILogger*, const char*> warning;
    ActionParams<ILogger*, const char*> error;
    ActionParams<ILogger*, const char*> fatal;
};

