#pragma once
#include <assert.h>
#include "../Allocators/Allocator.h"

/// <summary>
/// Stands for Constant Array , just a fancy wrapper over C-style arrays that includes the count and the type of the elements
/// </summary>
/// <typeparam name="T">The type of the elements in the array</typeparam>
template<typename T>
struct CArray
{
public:
    size_t count;
    Allocator alloc;
    T* data;

    static void Create( size_t count, CArray* out_arr, Allocator alloc, bool mem_init = true )
    {
        size_t total_size = (count * sizeof( T ));

        out_arr->data = alloc.alloc( alloc, total_size );
        out_arr->count = count;
        out_arr->alloc = alloc;

        if ( mem_init )
        {
            CoreContext::mem_init( out_arr->data, total_size )
        }
    }

    static void Destroy( CArray* in_arr )
    {
        if ( !in_arr->alloc.free )
            return;

        in_arr->alloc.free( in_arr->alloc, in_arr->data );
        in_arr->data = nullptr;
    }
};