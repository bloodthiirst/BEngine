#pragma once
#include "../../../Base/Filesystem/Filesystem.h"

class Win32Platform;

class Win32Filesystem : public Filesystem
{
public:
    Win32Platform* platform;
public:
    Win32Filesystem ( Win32Platform* platform );

    // Inherited via Filesystem
    virtual bool Exists ( const std::string& path ) const override;
    virtual bool Open ( const std::string& path, FileMode mode, bool binary, FileHandle* outFileHandle ) const override;
    virtual bool Close ( FileHandle& outFileHandle ) const override;
    virtual bool ReadText ( const FileHandle& outFileHandle, std::string& outString ) const override;
    virtual bool WriteText ( const FileHandle& outFileHandle, const std::string& outString ) const override;
    virtual bool Read ( const FileHandle& fileHandle, uint32_t size, void* const outBytes, uint32_t* outReadBytes ) const override;
    virtual bool ReadAll ( const FileHandle& fileHandle, void** outBytes, uint32_t* outReadBytes ) const override;
    virtual bool Write ( const FileHandle& fileHandle, uint32_t dataSize, void* const data, uint32_t* outWrittenBytes ) const override;
    virtual void Startup () override;
    virtual void Destroy () override;
};