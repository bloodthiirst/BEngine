#pragma once
#include <Typedefs/Typedefs.h>
#include <String/StringBuffer.h>
#include <String/StringView.h>

enum FileMode
{
    Read = 0x1,
    Write = 0x2,
    ReadWrite = 0x3
};

struct FileHandle
{
    void* handle;
    bool isValid;
};

struct Filesystem
{
    Func<bool, const StringView> exists;
    Func<bool, const StringView, FileMode, bool, FileHandle*> open;
    Func<bool, FileHandle*> close;
    Func<bool, const FileHandle*, StringBuffer*> read_text;
    Func<bool, const FileHandle, void** , uint64_t*> read_all;
    Func<bool,const FileHandle*, StringView> write_text;
};