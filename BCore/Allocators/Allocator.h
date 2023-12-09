#pragma once
#include <malloc.h>
#include <cstdint>
#include <crtdbg.h>

struct Allocator;

typedef void* (*Alloc)(Allocator, size_t);
typedef void* (*Realloc)(Allocator, void*, size_t);
typedef void (*Free)(Allocator, void*);

struct Allocator
{
    void* user_data;
    Alloc alloc;
    Realloc realloc;
    Free free;
};

struct Arena
{
    void* data;
    uint32_t capacity;
    uint32_t offset;
};

struct ArenaAllocator
{
public:
    static Allocator Create( Arena* arena )
    {
        Allocator alloc;
        alloc.user_data = arena;
        alloc.alloc = Allocate;
        alloc.realloc = nullptr;
        alloc.free = nullptr;

        return alloc;
    }

private:

    static void* NextPowerOfTwo( void* ptr, size_t size )
    {
        int64_t ptr_num = (int64_t)ptr;

        ptr_num += size;

        const int64_t mask = 0b11;
        const int64_t stride = 4;

        uint64_t res = ptr_num & mask;

        ptr_num += ((uint64_t)(res != 0)) * stride;

        return (void*)ptr_num;
    }


    static void* Allocate( Allocator alloc, size_t size )
    {
        Arena* arena = (Arena*)alloc.user_data;

        _ASSERT( arena->capacity < (arena->offset + size) );

        void* ptr = (void*)((size_t)arena->data + arena->offset);

        arena->offset += (uint32_t)size;

        return ptr;
    }

    static void* Reset( Arena* arena )
    {
        arena->offset = 0;
    }
};

/// <summary>
/// <para>A utility allocator that takes a pointer from the user and always returns it</para>
/// <para>Useful for situations where the memory is already allocated but we still need to pass an "Allocator" object to some method/function</para>
/// <para>In that case we just pass the pre-existing pointer through this "util" allocator</para>
/// <para>The most notable case to use this is when we want to put data into a chunck of memory allocated on the stack with "alloca"</para>
/// </summary>
struct EmplaceAllocator
{
public:
    static Allocator Create( void* ptr )
    {
        Allocator alloc;
        alloc.user_data = ptr;
        alloc.alloc = Allocate;
        alloc.realloc = nullptr;
        alloc.free = nullptr;

        return alloc;
    }
private:

    static void* Allocate( Allocator alloc, size_t size )
    {
        return alloc.user_data;
    }
};

struct HeapAllocator
{
public:
    static Allocator Create()
    {
        Allocator alloc;
        alloc.user_data = nullptr;
        alloc.alloc = Allocate;
        alloc.realloc = Reallocate;
        alloc.free = Free;

        return alloc;
    }
private:
    static void* Reallocate( Allocator alloc, void* ptr, size_t size )
    {
        return realloc( ptr, size );
    }

    static void* Allocate( Allocator alloc, size_t size )
    {
        return malloc( size );
    }

    static void Free( Allocator alloc, void* ptr )
    {
        free( ptr );
    }
};

/// <summary>
/// Returns stack allocated memory of size (size)
/// </summary>
#define STACK_ALLOC(size)  EmplaceAllocator::Create(alloca(size))