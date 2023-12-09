#pragma once
#include <memory>

struct Win32Memory
{
    static void Create(Memory* out_memory)
    {
        out_memory->malloc = malloc;
        out_memory->realloc = realloc;
        out_memory->free = free;
        out_memory->mem_compare = Win32MemCompare;
        out_memory->mem_copy = Win32MemCopy;
        out_memory->mem_init = Win32MemInit;
        out_memory->mem_set = Win32MemSet;
    }

    static bool Win32MemCompare( void* a, void* b, size_t size )
    {
        return memcmp(a , b , size) == 0;
    }

    static void Win32MemCopy(void* src, void* dst, size_t size)
    {
        memcpy(dst, src, size);
    }

    static void Win32MemInit(void* dst, size_t size)
    {
        memset(dst, 0, size);
    }

    static void Win32MemSet(void* dst, int32_t value , size_t size)
    {
        memset(dst, value, size);
    }
};

