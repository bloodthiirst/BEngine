#pragma once
#include <stdint.h>
#include <memory>
#include "../Typedefs/Typedefs.h"
#include "../Defines/Defines.h"

struct CORE_API Arena;

struct CORE_API CoreContext
{
    /// <summary>
    /// Allocate the specified amount of bytes from the heap and return a pointer to the allocated block
    /// </summary>
    static Func<void*, size_t> malloc;

    /// <summary>
    /// Take a preallocated block of heap memory and resize its size while keeping its data
    /// </summary>
    static Func<void*, void*, size_t> realloc;

    /// <summary>
    /// Free an allocated heap memory block
    /// </summary>
    static ActionParams<void*> free;

    /// <summary>
    /// Initialize a memory block to 0
    /// </summary>
    static ActionParams<void*, size_t> mem_init;

    /// <summary>
    /// Compare two memory blocks
    /// </summary>
    static Func<bool, void*, void*, size_t> mem_compare;

    /// <summary>
    /// set the value of all the bytes of memory block
    /// </summary>
    static ActionParams<void*, int32_t, size_t> mem_set;

    /// <summary>
    /// Copy a specified amount of data from "src" pointer to "dst" pointer
    /// </summary>
    static ActionParams<void*, void*, size_t> mem_copy;

    static Arena core_arena;

    static void DefaultContext();

private:
    static bool Win32MemCompare( void* a, void* b, size_t size )
    {
        return memcmp( a, b, size ) == 0;
    }

    static void Win32MemCopy( void* src, void* dst, size_t size )
    {
        memcpy( dst, src, size );
    }

    static void Win32MemInit( void* dst, size_t size )
    {
        memset( dst, 0, size );
    }

    static void Win32MemSet( void* dst, int32_t value, size_t size )
    {
        memset( dst, value, size );
    }
};