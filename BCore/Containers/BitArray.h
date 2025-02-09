#pragma once
#include "Allocators/Allocator.h"

struct BitArray
{
    char* data;
    size_t size;
    Allocator alloc;

    static void Create(BitArray* out_arr , size_t size , Allocator alloc)
    {
        size_t mem_size = (size / 8) + 1;
        
        *out_arr = {};
        out_arr->alloc = alloc;
        out_arr->data = (char*) ALLOC(out_arr->alloc, mem_size);
        out_arr->size = size;

        CoreContext::mem_init(out_arr->data , mem_size);
    }

    static void Destroy(BitArray* out_arr)
    {
        size_t mem_size = (out_arr->size / 8) + 1;
        FREE(out_arr->alloc , out_arr->data);
        *out_arr = {};
    }

    void Locate(size_t bit_pos , size_t* char_index , size_t* bit_index)
    {
        *char_index = bit_pos >> 3;
        *bit_index = bit_pos & 0b111;
    }

    bool Get(size_t bit_pos)
    {
        assert(bit_pos >= 0 && bit_pos < size);

        size_t char_index = bit_pos >> 3;
        size_t bit_index = bit_pos & 0b111;
        char bit_mask = (1 << bit_index);
        bool bit_val = data[char_index] & bit_mask;

        return bit_val;    
    }

    void Set(size_t bit_pos)
    {
        assert(bit_pos >= 0 && bit_pos < size);

        size_t char_index = bit_pos >> 3;
        size_t bit_index = bit_pos & 0b111;
        char bit_mask = (1 << bit_index);
        data[char_index] |= bit_mask;  
    }

    void Unset(size_t bit_pos)
    {
        assert(bit_pos >= 0 && bit_pos < size);

        size_t char_index = bit_pos >> 3;
        size_t bit_index = bit_pos & 0b111;
        char bit_mask = (1 << bit_index);
        data[char_index] &= ~bit_mask;  
    }

    void Toggle(size_t bit_pos)
    {
        assert(bit_pos >= 0 && bit_pos < size);

        size_t char_index = bit_pos >> 3;
        size_t bit_index = bit_pos & 0b111;
        char bit_mask = (1 << bit_index);
        data[char_index] ^= bit_mask;  
    }
};