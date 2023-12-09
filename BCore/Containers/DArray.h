#pragma once
#include <cstdlib>
#include "../Allocators/Allocator.h"

template<typename T>
class DArray
{
public:
    T* data;
    size_t size;
    size_t capacity;
    Allocator alloc;

    static void Create( size_t capacity, DArray* out_arr, Allocator alloc, bool mem_init = true )
    {
        out_arr->alloc = alloc;
        out_arr->data = (T*)alloc.alloc( alloc, capacity * sizeof( T ) );
        out_arr->capacity = capacity;
        out_arr->size = 0;

        if (mem_init)
        {
            memset( out_arr->data, 0, capacity * sizeof( T ) );
        }
    }

    static void Clear( DArray* in_arr )
    {
        in_arr->size = 0;
    }

    static void Destroy( DArray* in_arr )
    {
        if (!in_arr->alloc.free)
            return;

        in_arr->alloc.free( in_arr->alloc, in_arr->data );
        in_arr->data = nullptr;
    }

    static void Resize( DArray* in_arr, size_t new_size )
    {
        if (in_arr->alloc.realloc) {
            in_arr->data = (T*)in_arr->alloc.realloc( in_arr->alloc, in_arr->data, new_size * sizeof( T ) );
        }
        else
        {
            if (in_arr->alloc.free)
            {
                in_arr->alloc.free( in_arr->alloc, in_arr );
            }

            in_arr->data = (T*)in_arr->alloc.alloc( in_arr->alloc, new_size * sizeof( T ) );
        }

        in_arr->capacity = new_size;
    }

    static void Add( DArray* in_arr, T item )
    {
        if (in_arr->capacity < (in_arr->size + 1))
        {
            Resize( in_arr, (in_arr->capacity + 1) * 2 );
        }

        in_arr->data[in_arr->size++] = item;
    }

    static bool TryIndexOf( DArray* in_arr, size_t from, size_t to, T item, size_t* index_found )
    {
        for (; from < to; ++from)
        {
            if (in_arr->data[from] == item)
            {
                *index_found = from;
                return true;
            }
        }

        return false;
    }

    static size_t RemoveAll( DArray* in_arr, T item, Allocator temp_alloc )
    {
        // contains the deleted elements going from last to first
        size_t* indicies = (size_t*)temp_alloc.alloc( temp_alloc, sizeof( size_t ) * in_arr->size );
        size_t indicies_count = 0;

        for (size_t i = 0; i < in_arr->size; ++i)
        {
            if (in_arr->data[i] == item)
            {
                indicies[indicies_count++] = i;
            }
        }

        size_t acc = 0;
        for (size_t i = 0; i < indicies_count - 1; ++i)
        {
            size_t curr_i = indicies[i];
            size_t next_i = indicies[i + 1] - 1;

            for (size_t j = curr_i; j < next_i; j++)
            {
                in_arr->data[j - acc] = in_arr->data[j + 1];
            }

            acc++;
        }

        {
            size_t curr_i = indicies[indicies_count - 1];
            size_t next_i = in_arr->size - 1;

            for (size_t j = curr_i; j < next_i; j++)
            {
                in_arr->data[j - acc] = in_arr->data[j + 1];
            }
        }

        in_arr->size -= indicies_count;
        return indicies_count;
    }
};