#pragma once
#include "DArray.h"
#include "Stack.h"
#include "BitArray.h"

template <typename T>
struct SlotArray
{
    T* data;
    BitArray bit_arr;
    Stack<size_t> indicies_stack;
    size_t size;
    Allocator alloc;

    static bool Create(SlotArray* out_arr , size_t size , Allocator alloc)
    {
        *out_arr = {};
        out_arr->alloc = alloc;
        out_arr->size = size;

        out_arr.data = (T*) alloc.alloc(&out_arr->alloc , sizeof(T) * size);
        Stack<size_t>::Create(&out_arr->indicies_stack , size , alloc);
        BitArray::Create(&out_arr->bit_arr  , size , alloc);

        for(size_t i = 0; i < size; ++i)
        {
            Stack<size_t>::Push(&out_arr->indicies_stack , size - i - 1);
        }
    }

    size_t Add(T item)
    {
        assert(indicies_stack.size != 0);
        size_t idx = {};
        Stack<size_t>::Pop(&indicies_stack , &idx))
        
        assert(bit_arr.Get(idx) == false);

        bit_arr.Set(idx);
        data[idx] = item;
    }

    void RemoveAt(size_t idx)
    {
        assert(bit_arr.Get(idx) == true);
        bit_arr.Unset(idx);
        indicies_stack.Push(&indicies_stack , idx);
        data[idx] = {};
    }

    static void Destroy(SlotArray* out_arr)
    {
        out_arr->alloc.free(&out_arr->data, &out_arr->alloc);
        Stack<size_t>::Destroy(&out_arr->indicies_stack);
        BitArray::Destroy(&out_arr->bit_arr);
        *out_arr = {};
    }
};