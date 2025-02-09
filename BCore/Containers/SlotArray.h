#pragma once
#include "DArray.h"
#include "Stack.h"
#include "BitArray.h"

template <typename T>
struct SlotArray
{
    struct SlotArrayElem
    {
        size_t idx;
        T item;
    };
    
    T* data;
    BitArray bit_arr;
    Stack<size_t> indicies_stack;
    size_t size;
    size_t capacity;
    Allocator alloc;

    static void Create(SlotArray* out_arr , size_t capacity , Allocator alloc)
    {
        *out_arr = {};
        out_arr->alloc = alloc;
        out_arr->size = 0;
        out_arr->capacity = capacity;
        out_arr->data = (T*) ALLOC(out_arr->alloc , sizeof(T) * capacity);
        
        Stack<size_t>::Create(&out_arr->indicies_stack , capacity , alloc);
        BitArray::Create(&out_arr->bit_arr  , capacity , alloc);

        for(size_t i = 0; i < capacity; ++i)
        {
            Stack<size_t>::Push(&out_arr->indicies_stack , capacity - i - 1);
        }
    }

    void GetAll(DArray<SlotArrayElem>* in_arr)
    {
        assert(in_arr->size == 0);
        if(in_arr->capacity < size)
        {
            DArray<SlotArrayElem>::Resize(in_arr , size);
        }

        size_t i = 0;
        while( i < bit_arr.size || in_arr->size != size)
        {
            if(bit_arr.Get(i))
            {
                SlotArrayElem elem = {};
                elem.idx = i;
                elem.item = data[i];

                DArray<SlotArrayElem>::Add(in_arr , elem);
            }

            i++;
        }
    }

    size_t Add(T item)
    {
        assert(indicies_stack.size != 0);
        size_t idx = {};
        Stack<size_t>::Pop(&indicies_stack , &idx);
        
        assert(bit_arr.Get(idx) == false);

        bit_arr.Set(idx);
        data[idx] = item;
        size++;

        return idx;
    }

    bool HasIndex(size_t idx)
    {
        return bit_arr.Get(idx);
    }

    void RemoveAt(size_t idx)
    {
        assert(HasIndex(idx));
        bit_arr.Unset(idx);
        indicies_stack.Push(&indicies_stack , idx);
        size--;
        
        data[idx] = {};
    }

    static void Destroy(SlotArray* out_arr)
    {
        FREE(out_arr->alloc, out_arr->data);
        Stack<size_t>::Destroy(&out_arr->indicies_stack);
        assert(CoreContext::mem_compare(&out_arr->indicies_stack , &Stack<size_t>{} , sizeof(Stack<size_t>)));
        
        BitArray::Destroy(&out_arr->bit_arr);
        assert(CoreContext::mem_compare(&out_arr->bit_arr , &BitArray{} , sizeof(BitArray)));
        
        *out_arr = {};
    }
};