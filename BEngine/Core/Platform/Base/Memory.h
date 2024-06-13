#pragma once
#include <Typedefs/Typedefs.h>
#include <cstdint>

struct Memory
{
    /// <summary>
    /// Allocate the specified amount of bytes from the heap and return a pointer to the allocated block
    /// </summary>
    Func<void*, size_t> malloc;

    /// <summary>
    /// Take a preallocated block of heap memory and resize its size while keeping its data
    /// </summary>
    Func<void*, void*, size_t> realloc;

    /// <summary>
    /// Free an allocated heap memory block
    /// </summary>
    ActionParams<void*> free;

    /// <summary>
    /// Initialize a memory block to 0
    /// </summary>
    ActionParams<void*, size_t> mem_init;

    /// <summary>
    /// Compare two memory blocks
    /// </summary>
    Func<bool, void*, void*, size_t> mem_compare;

    /// <summary>
    /// set the value of all the bytes of memory block
    /// </summary>
    ActionParams<void*, int32_t, size_t> mem_set;

    /// <summary>
    /// <para>Copy a specified amount of data from "src" pointer to "dst" pointer</para>
    /// <para>NOTE : avoid using in case of overlap between the src and dst memory</para>
    /// </summary>
    static ActionParams<void*, void*, size_t> mem_copy;

    /// <summary>
    /// <para>Move a specified amount of data from "src" pointer to "dst" pointer</para>   
    /// <para>NOTE : Use in case of overlap between the src and dst memory</para>
    /// </summary>
    static ActionParams<void*, void*, size_t> mem_move;
};