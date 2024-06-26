#pragma once
#include <Typedefs/Typedefs.h>
#include <String/StringBuffer.h>
#include <String/StringView.h>

enum FileModeFlag
{
    Read        = 0b01,
    Write       = 0b10,
    ReadWrite   = 0b11
};

struct FileHandle
{
    void* handle;
    bool is_valid;
};

struct Filesystem
{
    Func<bool, FileHandle*> close;
    Func<bool, const StringView> exists;
    Func<bool, const StringView, FileModeFlag, bool, FileHandle*> open;
    Func<bool, const FileHandle*, size_t*> get_size;
    Func<bool, const FileHandle*, StringBuffer*> read_text;
    Func<bool, const FileHandle, void* , uint64_t*> read_all;
    Func<bool, const FileHandle*, StringView> write_text;
};