#pragma once
#include <string>

class Application;

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

class Filesystem
{
public:
    Application* app;
    void virtual Startup () = 0;
    void virtual Destroy () = 0;

    bool virtual Exists ( const std::string& path ) const = 0;
    bool virtual Open ( const std::string& path , FileMode mode , bool binary, FileHandle* outFileHandle ) const = 0;
    bool virtual Close (FileHandle& outFileHandle ) const = 0;
    bool virtual ReadText (const FileHandle& outFileHandle , std::string& outString ) const = 0;
    bool virtual WriteText (const FileHandle& outFileHandle ,const std::string& outString ) const = 0;
    bool virtual Read ( const FileHandle& fileHandle, uint32_t size, void* const outBytes, uint32_t* outReadBytes ) const = 0;
    bool virtual ReadAll ( const FileHandle& fileHandle, void** outBytes, uint32_t* outReadBytes ) const = 0;
    bool virtual Write ( const FileHandle& fileHandle, uint32_t dataSize, void* const data, uint32_t* outWrittenBytes ) const = 0;


};