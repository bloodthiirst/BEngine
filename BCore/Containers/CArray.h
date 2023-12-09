#pragma once
#include <cstdlib>
#include <assert.h>
#include "../Allocators/Allocator.h"

template<typename T>
class CArray
{
public:
    size_t size;
    size_t capacity;
    Allocator alloc;
    T data[1];


    static void Create(size_t capacity, CArray* out_arr , Allocator alloc)
    {
        ++capacity;
        size_t total_size = (sizeof(size_t) * 2) + sizeof(Allocator) + ( capacity * sizeof(T));

        *out_arr =  *((CArray*)alloc.alloc(alloc, total_size));
        
        out_arr->size = 0;
        out_arr->capacity = capacity;
        out_arr->alloc = alloc;
    }

    static void Destroy(CArray** in_arr)
    {
        if (!(*in_arr)->alloc.free)
            return;

        (*in_arr)->alloc.free((**in_arr).alloc , *in_arr);
        *in_arr = nullptr;
    }

    static void Add(CArray* in_arr, T item)
    {
        assert(in_arr->capacity >= (in_arr->size + 1));
        in_arr->data[in_arr->size++] = item;
    }

    static bool TryIndexOf(CArray* in_arr, size_t from, size_t to, T item, size_t* index_found)
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

    static size_t RemoveAll(CArray* in_arr, T item , Allocator temp_alloc)
    {
        // contains the deleted elements going from last to first
        size_t* indicies = (size_t*)temp_alloc.alloc(temp_alloc , sizeof(size_t) * in_arr->size);
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