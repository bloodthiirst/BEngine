#pragma once
#include <assert.h>
#include "../Allocators/Allocator.h"

template <typename T>
struct Queue
{
    T *data;
    size_t size;
    size_t capacity;
    Allocator alloc;
    size_t enqueue_index;
    size_t dequeque_index;

    static void Create(Queue *out_queue, size_t capacity, Allocator alloc)
    {
        assert(capacity != 0);
        *out_queue = {};
        out_queue->capacity = capacity;
        out_queue->alloc = alloc;
        out_queue->data = (T *)out_queue->alloc.alloc(&out_queue->alloc, sizeof(T) * capacity);
    }

    static void Destroy(Queue *out_queue)
    {
        out_queue->alloc.free(&out_queue->alloc, out_queue->data);
        *out_queue = {};
    }

    static void Resize(Queue *inout_queue, size_t new_capacity)
    {
        assert(new_capacity > inout_queue->capacity);

        if (inout_queue->alloc.realloc)
        {
            inout_queue->data = (T *)inout_queue->alloc.realloc(&inout_queue->alloc, inout_queue->data, new_capacity);
        }
        else
        {
            T* old_data = inout_queue->data;
            T* new_data = (T *)inout_queue->alloc.alloc(&inout_queue->alloc ,  new_capacity * sizeof(T));
            CoreContext::mem_copy(old_data, new_data, inout_queue->size * sizeof(T));

            if (inout_queue->alloc.free)
            {
                inout_queue->alloc.free(&inout_queue->alloc, old_data);
            }

            inout_queue->data = new_data;
        }

        inout_queue->capacity = new_capacity;
    }

    static bool TryDequeue(Queue* inout_queue , T* out_item)
    {
        if(inout_queue->size == 0)
        {
            *out_item = nullptr;
            return false;
        }

        Dequeue(inout_queue , out_item);
        return true;
    }

    static void Dequeue(Queue* inout_queue , T* out_item)
    {
        assert(inout_queue->size != 0);

        *out_item = inout_queue->data[inout_queue->dequeque_index];
        inout_queue->dequeque_index++;
        size--;
    }

    static void Enqueue(Queue *inout_queue, T item)
    {
        // if we don't have enough place to enqueue past the enqueue index
        // BUT we still have place empty space before the dequeue index
        // THEN we shift the whole queue to the left
        if(inout_queue->enqueue_index == (inout_queue->capacity - 1) && inout_queue->dequeque_index > 0)
        {
            T* from = inout_queue->data;
            T* to = inout_queue->data - enqueue_index;
            size_t mem_size = sizeof(T) * inout_queue->size;
            CoreContext::mem_copy(from , to , mem_size );
            inout_queue->enqueue_index -= inout_queue->dequeque_index;
            inout_queue->dequeque_index = 0;
        }

        if ((size + 1) < inout_queue->capacity)
        {
            size_t new_capacity = (inout_queue->capacity + 1) * 2;
            Resize(inout_queue, new_capacity);
        }

        inout_queue->data[inout_queue->enqueue_index] = item;
        inout_queue->enqueue_index++;
        inout_queue->size++;
    }
};