#pragma once
#include <malloc.h>
#include <cstdint>
#include <crtdbg.h>
#include "../Context/CoreContext.h"
struct Allocator;

typedef void* (*Alloc)(Allocator, size_t);
typedef void* (*Realloc)(Allocator, void*, size_t);
typedef void (*Free)(Allocator, void*);

struct CORE_API Allocator
{
    void* user_data;
    Alloc alloc;
    Realloc realloc;
    Free free;
};

struct CORE_API Arena
{
    void* data;
    size_t capacity;
    size_t offset;

    static Arena Create( size_t capacity, bool init = true )
    {
        Arena res = {};
        res.data = CoreContext::malloc( capacity );
        res.capacity = capacity;

        if ( init )
        {
            CoreContext::mem_init( res.data, capacity );
        }

        return res;
    }

    static Arena CreateSubArena( Arena* source )
    {
        Arena sub = {};
        sub.data = ((char*) source->data) + source->offset;
        sub.offset = 0;
        sub.capacity = source->capacity - source->offset;

        return sub;
    }

    static void Reset( Arena* arena )
    {
        arena->offset = 0;
    }
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
        int64_t ptr_num = (int64_t) ptr;

        ptr_num += size;

        const size_t mask = 0b11;
        const size_t stride = 4;

        size_t res = ptr_num & mask;

        ptr_num += ((size_t) (res != 0)) * stride;

        return (void*) ptr_num;
    }


    static void* Allocate( Allocator alloc, size_t size )
    {
        Arena* arena = (Arena*) alloc.user_data;

        _ASSERT( (arena->offset + size) < arena->capacity );

        void* ptr = (void*) ((char*) arena->data + arena->offset);

        arena->offset += size;

        return ptr;
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
        return CoreContext::realloc( ptr, size );
    }

    static void* Allocate( Allocator alloc, size_t size )
    {
        return CoreContext::malloc( size );
    }

    static void Free( Allocator alloc, void* ptr )
    {
        CoreContext::free( ptr );
    }
};

/// <summary>
/// Returns stack allocated memory of size (size)
/// </summary>
#define STACK_ALLOC(size)  EmplaceAllocator::Create(alloca(size))

/// <summary>
/// Returns stack allocated memory of size (size) and of type (type)
/// </summary>
#define STACK_ALLOC_ARRAY(type , count) EmplaceAllocator::Create(alloca(sizeof(type) * count))