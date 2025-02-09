#pragma once
#include "../Allocators/Allocator.h"

template <typename T>
struct Stack
{
    T *data;
    size_t size;
    size_t capacity;
    Allocator alloc;

    static void Create(Stack *out_stack, size_t capacity, Allocator alloc)
    {
        assert(capacity != 0);
        *out_stack = {};
        out_stack->capacity = capacity;
        out_stack->alloc = alloc;
        out_stack->data = (T*) ALLOC(out_stack->alloc, sizeof(T) * capacity);
    }

    static bool TryPeek(Stack *in_stack, T *out_item)
    {
        assert(out_item != nullptr);
        
        if (in_stack->size == 0)
        {
            *out_item = {};
            return false;
        }

        *out_item = in_stack->data[in_stack->size - 1];

        return true;
    }

    static void Peek(Stack* in_stack , T* out_item)
    {
        assert(in_stack->size > 0);
        *out_item = in_stack->data[in_stack->size - 1];
    }

    static void Pop(Stack* in_stack , T* out_item)
    {
        assert(in_stack->size > 0);
        
        if (out_item)
        {
            *out_item = in_stack->data[in_stack->size - 1];
        }

        in_stack->size--;
    }

    static bool TryPop(Stack *in_stack, T *out_item)
    {
        if (in_stack->size == 0)
        {
            *out_item = {};
            return false;
        }

        if (out_item)
        {
            *out_item = in_stack->data[in_stack->size - 1];
        }

        in_stack->size--;

        return true;
    }

    static void Resize(Stack *inout_stack, size_t new_capacity)
    {
        assert(new_capacity > inout_stack->size + 1);

        if (inout_stack->alloc.realloc)
        {
            inout_stack->data = (T *) REALLOC(inout_stack->alloc, inout_stack->data, new_capacity * sizeof(T));
        }
        else
        {
            T *old_data = inout_stack->data;
            T *new_data = (T *)ALLOC(inout_stack->alloc, new_capacity * sizeof(T));
            CoreContext::mem_copy(old_data, new_data, inout_stack->size * sizeof(T));

            if (inout_stack->alloc.free)
            {
                FREE(inout_stack->alloc, old_data);
            }

            inout_stack->data = new_data;
        }

        inout_stack->capacity = new_capacity;
    }

    static void Push(Stack *in_stack, T item)
    {
        const size_t resize_factor = 2;
        if ((in_stack->size + 1) > in_stack->capacity)
        {
            Resize(in_stack , (in_stack->capacity + 1) * resize_factor);
        }

        in_stack->data[in_stack->size] = item;
        in_stack->size++;
    }

    static void Destroy(Stack *out_stack)
    {
        FREE(out_stack->alloc, out_stack->data);
        *out_stack = {};
    }
};