#include "../../../Base/Filesystem/Filesystem.h"
#include "Win32Filesystem.h"
#include <sys/stat.h>
#include "../Platform/Win32Platform.h"
#include <direct.h> // _getcwd
#pragma warning(disable : 4996)

Win32Filesystem::Win32Filesystem ( Win32Platform* platform )
{
    this->platform = platform;
    this->app = platform->app;
}


bool Win32Filesystem::Exists ( const std::string& path ) const
{
    struct stat buffer;
    return stat ( path.c_str (), &buffer );
}

bool Win32Filesystem::Open ( const std::string& path, FileMode mode, bool binary, FileHandle* outFileHandle ) const
{
    outFileHandle->isValid = false;
    outFileHandle->handle = 0;

    std::string strMode = {};

    switch ( mode )
    {
        case FileMode::ReadWrite:
        {
            strMode = binary ? "w+b" : "w+";
            break;
        }
        case FileMode::Read:
        {
            strMode = binary ? "rb" : "r";
            break;
        }
        case FileMode::Write:
        {
            strMode = binary ? "wb" : "w";
            break;
        }
    }

    FILE* fHandle = fopen ( path.c_str (), strMode.c_str () );

    if ( !fHandle )
        return false;

    outFileHandle->handle = fHandle;
    outFileHandle->isValid = true;

    return true;
}

bool Win32Filesystem::Close ( FileHandle& outFileHandle ) const
{
    if ( outFileHandle.isValid )
    {
        fclose ( (FILE*) outFileHandle.handle );
        outFileHandle.isValid = false;
        return true;
    }

    return false;
}

bool Win32Filesystem::ReadText ( const FileHandle& fileHandle, std::string& outString ) const
{
    if ( !fileHandle.handle || !fileHandle.isValid )
        return false;

    FILE* fHandle = (FILE*) fileHandle.handle;
    fseek ( fHandle, 0L, SEEK_END );
    int fileSize = ftell ( fHandle );
    rewind ( fHandle );

    char* strBuffer = (char*) _malloca ( sizeof ( char ) * fileSize );
    fgets ( strBuffer, fileSize, fHandle );

    int stringLength = strlen ( strBuffer );

    outString = std::string ( strBuffer, stringLength );

    return true;
}

bool Win32Filesystem::WriteText ( const FileHandle& fileHandle, const std::string& outString ) const
{
    if ( !fileHandle.handle || !fileHandle.isValid )
        return false;

    FILE* fHandle = (FILE*) fileHandle.handle;

    int result = fputs ( outString.c_str (), fHandle );

    fflush ( fHandle );

    return result != EOF;
}

bool Win32Filesystem::Read ( const FileHandle& fileHandle, uint32_t bytesToRead, void* const outBytes, uint32_t* outReadBytes ) const
{
    if ( !fileHandle.handle || !fileHandle.isValid || !outBytes )
        return false;

    *outReadBytes = fread ( outBytes, 1, bytesToRead, (FILE*) fileHandle.handle );

    return *outReadBytes != bytesToRead;
}

bool Win32Filesystem::ReadAll ( const FileHandle& fileHandle, void** outBytes, uint32_t* outReadBytes ) const
{
    if ( !fileHandle.handle || !fileHandle.isValid || !outBytes )
        return false;

    FILE* fHandle = (FILE*) fileHandle.handle;
    fseek ( fHandle, 0L, SEEK_END );
    int fileSize = ftell ( fHandle );
    rewind ( fHandle );

    *outBytes = malloc ( fileSize );

    *outReadBytes = fread ( *outBytes, 1, fileSize, fHandle );

    return *outReadBytes == fileSize;
}

bool Win32Filesystem::Write ( const FileHandle& fileHandle, uint32_t dataSize, void* const data, uint32_t* outWrittenBytes ) const
{
    if ( !fileHandle.handle || !fileHandle.isValid || !data )
        return false;

    FILE* fHandle = (FILE*) fileHandle.handle;

    *outWrittenBytes = fwrite ( data, 1, dataSize, fHandle );

    if ( *outWrittenBytes != dataSize )
        return false;

    fflush ( fHandle );

    return true;
}

void Win32Filesystem::Startup ()
{}

void Win32Filesystem::Destroy ()
{}
