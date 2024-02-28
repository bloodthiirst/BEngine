#pragma once
#include <String/StringView.h>
#include <Typedefs/Typedefs.h>

struct ILogger
{
    void* user_data;
    ActionParams<ILogger*, StringView> log;
    ActionParams<ILogger*, StringView> info;
    ActionParams<ILogger* , size_t> new_line;
    ActionParams<ILogger*, StringView> warning;
    ActionParams<ILogger*, StringView> error;
    ActionParams<ILogger*, StringView> fatal;
};

