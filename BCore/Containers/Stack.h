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
        out_stack->data = (T*) out_stack->alloc.alloc(&out_stack->alloc, sizeof(T) * capacity);
    }

    static bool TryPeek(Stack *in_stack, T *out_item)
    {
        assert(out_item);
        
        if (in_stack->size == 0)
        {
            *out_item = {};
            return false;
        }

        *out_item = in_stack->data[in_stack->size - 1];

        return true;
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

    static void Push(Stack *in_stack, T item)
    {
        const float resize_factor = 2;

        if ((in_stack->size + 1) > in_stack->capacity)
        {
            const size_t new_size = (size_t)(in_stack->capacity * resize_factor);
            assert(new_size > in_stack->size + 1);

            if (in_stack->alloc.realloc)
            {
                in_stack->data = (T*) in_stack->alloc.realloc(&in_stack->alloc, in_stack->data, new_size);
            }
            else
            {
                T *old_data = in_stack->data;
                T *new_data = (T*) in_stack->alloc.alloc(&in_stack->alloc, new_size);
                if (in_stack->alloc.free)
                {
                    in_stack->alloc.free(&in_stack->alloc, old_data);
                }

                in_stack->data = new_data;
            }
        }

        in_stack->data[in_stack->size] = item;
        in_stack->size++;
    }

    static void Destroy(Stack *out_stack)
    {
        out_stack->alloc.free(&out_stack->alloc, out_stack->data);
        *out_stack = {};
    }
};