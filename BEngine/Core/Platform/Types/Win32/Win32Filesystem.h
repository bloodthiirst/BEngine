#include "../../Base/Filesystem.h"
#include <sys/stat.h>
#include <cstdio>
#pragma warning(disable : 4996)

struct Win32Filesystem
{
public:
    static void Create(Filesystem* out_filesystem)
    {
        out_filesystem->exists = Win32Exists;
        out_filesystem->open = Win32Open;
        out_filesystem->close = Win32Close;
        out_filesystem->read_text = ReadText;
        out_filesystem->write_text = WriteText;
        out_filesystem->read_all = ReadAll;
    }
private :
    static bool Win32Exists(StringView path)
    {
        struct stat buffer;

        char* path_mem = (char*) alloca(path.length + 1);     
        Allocator alloc = EmplaceAllocator::Create(path_mem);
        StringView::ToCString(path, alloc);

        return stat(path_mem, &buffer);
    }

    static bool Win32Open(StringView path, FileMode mode, bool binary, FileHandle* outFileHandle)
    {
        outFileHandle->isValid = false;
        outFileHandle->handle = 0;

        const char* str_mode = nullptr;

        switch (mode)
        {
        case FileMode::ReadWrite:
        {
            str_mode = binary ? "w+b" : "w+";
            break;
        }
        case FileMode::Read:
        {
            str_mode = binary ? "rb" : "r";
            break;
        }
        case FileMode::Write:
        {
            str_mode = binary ? "wb" : "w";
            break;
        }
        }

        char* path_mem = (char*)alloca(path.length + 1);
        Allocator alloc = EmplaceAllocator::Create(path_mem);          
        StringView::ToCString(path, alloc);

        FILE* fHandle = fopen(path_mem, str_mode);

        if (!fHandle)
            return false;

        outFileHandle->handle = fHandle;
        outFileHandle->isValid = true;

        return true;
    }

    static bool Win32Close(FileHandle* outFileHandle)
    {
        if (outFileHandle->isValid)
        {
            fclose((FILE*)outFileHandle->handle);
            outFileHandle->isValid = false;
            return true;
        }

        return false;
    }

    static bool ReadText(const FileHandle* file_handle, StringBuffer* out_string)
    {
        if (!file_handle->handle || !file_handle->isValid)
            return false;

        FILE* handle = (FILE*)file_handle->handle;
        fseek(handle, 0L, SEEK_END);
        int file_size = ftell(handle);
        rewind(handle);

        StringBuffer::Resize(out_string, file_size);
        fgets(out_string->buffer, file_size, handle);

        return true;
    }

    static bool ReadAll( const FileHandle fileHandle, void** outBytes, uint64_t* outReadBytes )
    {
        if (!fileHandle.handle || !fileHandle.isValid || !outBytes)
            return false;

        FILE* fHandle = (FILE*)fileHandle.handle;
        fseek( fHandle, 0L, SEEK_END );
        int fileSize = ftell( fHandle );
        rewind( fHandle );

        *outBytes = malloc( fileSize );
        *outReadBytes = fread( *outBytes, 1, fileSize, fHandle );

        return *outReadBytes == fileSize;
    }

    static bool WriteText(const FileHandle* file_handle, StringView in_string)
    {
        if (!file_handle->handle || !file_handle->isValid)
            return false;

        FILE* handle = (FILE*)file_handle->handle;

        // TODO : refactor this to allocate from some global temporary memory instead of the stack since file can be reaaaaaaaaaally big
        char* temp_str = (char*) alloca((in_string.length + 1) * sizeof(char));
        
        Allocator alloc = EmplaceAllocator::Create(temp_str);
        
        StringView::ToCString(in_string, alloc);

        int result = fputs(temp_str, handle);

        fflush(handle);

        return result != EOF;
    }

    static bool Read(const FileHandle& fileHandle, size_t bytesToRead, void* const outBytes, size_t* outReadBytes)
    {
        if (!fileHandle.handle || !fileHandle.isValid || !outBytes)
            return false;

        *outReadBytes = fread(outBytes, 1, bytesToRead, (FILE*)fileHandle.handle);

        return *outReadBytes != bytesToRead;
    }


    static bool Write(const FileHandle& fileHandle, size_t dataSize, void* const data, size_t* outWrittenBytes)
    {
        if (!fileHandle.handle || !fileHandle.isValid || !data)
            return false;

        FILE* fHandle = (FILE*)fileHandle.handle;

        *outWrittenBytes = fwrite(data, 1, dataSize, fHandle);

        if (*outWrittenBytes != dataSize)
            return false;

        fflush(fHandle);

        return true;
    }

};
