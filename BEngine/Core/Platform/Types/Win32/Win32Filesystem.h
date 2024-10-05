#include "../../Base/Filesystem.h"
#include "../../../Global/Global.h"
#include <sys/stat.h>
#include <cstdio>
#pragma warning(disable : 4996)

struct Win32Filesystem
{
public:
    static void Create(Filesystem *out_filesystem)
    {
        out_filesystem->exists = Win32Exists;
        out_filesystem->open = Win32Open;
        out_filesystem->close = Win32Close;
        out_filesystem->read_text = ReadText;
        out_filesystem->write_text = WriteText;
        out_filesystem->write_bytes = WriteBytes;
        out_filesystem->read_all = ReadAll;
        out_filesystem->get_size = Win32GetSize;
        out_filesystem->get_files = Win32GetFiles;
        out_filesystem->get_directories = Win32GetDirectories;
    }

private:
    static void Win32GetDirectories(StringView path, DArray<StringBuffer> *inout_sub_files, Allocator alloc)
    {
        {
            StringBuffer fmt = StringUtils::Format(Global::alloc_toolbox.frame_allocator, "{}\\*", path);
            char *str = StringView::ToCString(fmt.view, Global::alloc_toolbox.frame_allocator);

            WIN32_FIND_DATA find_data;
            HANDLE file_h = FindFirstFile(str, &find_data); // FILES

            while (FindNextFile(file_h, &find_data) != 0)
            {
                // if is not directory , skip
                if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
                {
                    continue;
                }

                // if directory is referencing itself "..", skip
                if (find_data.cFileName[0] == '.' && find_data.cFileName[1] == '.')
                {
                    continue;
                }

                StringBuffer file_name = StringBuffer::Create(find_data.cFileName, alloc);
                DArray<StringBuffer>::Add(inout_sub_files, file_name);
            }

            FindClose(file_h);
        }
    }

    static void Win32GetFiles(StringView path, DArray<StringBuffer> *inout_sub_files, Allocator alloc)
    {
        StringBuffer fmt = StringUtils::Format(Global::alloc_toolbox.frame_allocator, "{}\\*", path);
        char *str = StringView::ToCString(fmt.view, Global::alloc_toolbox.frame_allocator);

        WIN32_FIND_DATA find_data;
        HANDLE file_h = FindFirstFile(str, &find_data); // FILES

        while (FindNextFile(file_h, &find_data) != 0)
        {
            // if is not normal file , skip
            if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) != FILE_ATTRIBUTE_ARCHIVE)
            {
                continue;
            }

            StringBuffer directory_name = StringBuffer::Create(find_data.cFileName, alloc);
            DArray<StringBuffer>::Add(inout_sub_files, directory_name);
        }

        FindClose(file_h);
    }

    static bool Win32GetSize(const FileHandle *fhandle, size_t *out_size)
    {
        FILE *handle = (FILE *)fhandle->handle;

        int32_t prev = ftell(handle);
        fseek(handle, 0L, SEEK_END);
        *out_size = (size_t)ftell(handle);
        fseek(handle, prev, SEEK_SET); // go back to where we were

        return true;
    }

    static bool Win32Exists(StringView path)
    {
        struct stat buffer;

        char *path_mem = (char *)alloca(path.length + 1);
        Allocator alloc = EmplaceAllocator::Create(path_mem);
        StringView::ToCString(path, alloc);

        return stat(path_mem, &buffer);
    }

    static bool Win32Open(StringView path, FileModeFlag mode, bool binary, FileHandle *outFileHandle)
    {
        outFileHandle->is_valid = false;
        outFileHandle->handle = 0;

        const char *str_mode = "";

        switch (mode)
        {
        case FileModeFlag::ReadWrite:
        {
            str_mode = binary ? "w+b" : "w+";
            break;
        }
        case FileModeFlag::Read:
        {
            str_mode = binary ? "rb" : "r";
            break;
        }
        case FileModeFlag::Write:
        {
            str_mode = binary ? "wb" : "w";
            break;
        }
        }

        char *path_mem = (char *)alloca(path.length + 1);
        Allocator alloc = EmplaceAllocator::Create(path_mem);
        char *pathC = StringView::ToCString(path, alloc);

        FILE *fHandle = fopen(pathC, str_mode);

        if (!fHandle)
            return false;

        outFileHandle->handle = fHandle;
        outFileHandle->is_valid = true;

        return true;
    }

    static bool Win32Close(FileHandle *outFileHandle)
    {
        if (outFileHandle->is_valid)
        {
            fclose((FILE *)outFileHandle->handle);
            outFileHandle->is_valid = false;
            return true;
        }

        return false;
    }

    static bool ReadText(const FileHandle *file_handle, StringBuffer *out_string)
    {
        if (!file_handle->handle || !file_handle->is_valid)
            return false;

        FILE *handle = (FILE *)file_handle->handle;
        fseek(handle, 0L, SEEK_END);
        int file_size = ftell(handle);
        rewind(handle);

        StringBuffer::Resize(out_string, file_size);
        fgets(out_string->buffer, file_size, handle);

        return true;
    }

    static bool ReadAll(const FileHandle fileHandle, void *outBytes, uint64_t *outReadBytes)
    {
        if (!fileHandle.handle || !fileHandle.is_valid || !outBytes)
            return false;

        FILE *fHandle = (FILE *)fileHandle.handle;
        fseek(fHandle, 0L, SEEK_END);
        int fileSize = ftell(fHandle);
        rewind(fHandle);

        *outReadBytes = fread(outBytes, 1, fileSize, fHandle);

        return *outReadBytes == fileSize;
    }


    static bool WriteBytes(const FileHandle *file_handle, ArrayView<char> in_bytes)
    {
        if (!file_handle->handle || !file_handle->is_valid)
            return false;

        FILE *handle = (FILE *)file_handle->handle;

        size_t result = fwrite(in_bytes.data , sizeof(byte) , in_bytes.size , handle);

        fflush(handle);

        return result != EOF;
    }

    static bool WriteText(const FileHandle *file_handle, StringView in_string)
    {
        if (!file_handle->handle || !file_handle->is_valid)
            return false;

        FILE *handle = (FILE *)file_handle->handle;
        
        size_t result = fwrite(in_string.buffer , sizeof(char) , in_string.length , handle);

        fseek(handle , in_string.length , SEEK_SET);

        result = fwrite("\0" , sizeof(char) , 1 , handle);

        fflush(handle);

        return result != EOF;
    }

    static bool Read(const FileHandle &fileHandle, size_t bytesToRead, void *const outBytes, size_t *outReadBytes)
    {
        if (!fileHandle.handle || !fileHandle.is_valid || !outBytes)
            return false;

        *outReadBytes = fread(outBytes, 1, bytesToRead, (FILE *)fileHandle.handle);

        return *outReadBytes != bytesToRead;
    }

    static bool Write(const FileHandle &fileHandle, size_t dataSize, void *const data, size_t *outWrittenBytes)
    {
        if (!fileHandle.handle || !fileHandle.is_valid || !data)
            return false;

        FILE *fHandle = (FILE *)fileHandle.handle;

        *outWrittenBytes = fwrite(data, 1, dataSize, fHandle);

        if (*outWrittenBytes != dataSize)
            return false;

        fflush(fHandle);

        return true;
    }
};
