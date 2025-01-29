#include "DArray.h"
#include "Typedefs/Typedefs.h"

template <typename T>
struct MinHeap
{
    DArray<T> data;
    Func<size_t , T> priorityFnPtr;

private:
    size_t GetFirstChildIndex(size_t index)
    {
        assert(index < data.size && index >= 0);
        return (index * 2) + 1;
    }

    size_t GetParentIndex(size_t index)
    {
        assert(index < data.size && index >= 0);
        return (index - 1) / 2;
    }

    bool HasChildren(size_t index)
    {
        assert(index < data.size && index >= 0);

        size_t first_child_idx = GetFirstChildIndex(index);

        return first_child_idx < data.size;
    }

    bool HasParent(size_t index)
    {
        assert(index < data.size && index >= 0);
         
        if(index == 0)
        {
            return false;
        }

        size_t parent_idx = GetParentIndex(index);

        return parent_idx >= 0;
    }

public:

    static void Create(Allocator alloc , MinHeap* out_heap, size_t capacity, Func<size_t , T> priorityFnPtr )
    {
        assert(priorityFnPtr != nullptr);

        MinHeap heap = {};
        heap.priorityFnPtr = priorityFnPtr;
        DArray<T>::Create(capacity , &heap.data , alloc);

        *out_heap = heap;
    }

    static void Destroy(MinHeap* heap)
    {
        DArray<T>::Destroy(&heap->data);
        *heap = {};
    }

    T Pop()
    {
        assert(data.size != 0);

        T min_val = data.data[0];

        // swap
        data.data[0] = data.data[data.size - 1];
        DArray<T>::RemoveAt(&data, data.size - 1);

        size_t idx = 0;
        while (HasChildren(idx))
        {
            size_t first_idx = GetFirstChildIndex(idx);
            size_t second_idx = first_idx + 1;

            size_t min_idx = first_idx;
            size_t min_prio = priorityFnPtr(data.data[first_idx]);

            if (second_idx < data.size)
            {
                size_t second_prio = priorityFnPtr(data.data[second_idx]);
                
                if(second_prio < min_prio)
                {
                    min_idx = second_idx;
                    min_prio = second_prio;
                }
            }

            size_t curr_prio = priorityFnPtr(data.data[idx]);
            if ( curr_prio < min_prio)
            {
                break;
            }
            
            data.data[idx] = data.data[min_idx];
            idx = min_idx;
        }

        return min_val;
    }

    T Peek()
    {
        assert(data.size != 0);
        return data.data[0];
    }

    void Add(T item)
    {
        size_t idx = data.size;
        DArray<T>::Add(&data, item);

        while (HasParent(idx))
        {
            size_t parent_idx = GetParentIndex(idx);

            size_t curr_val = priorityFnPtr(data.data[idx]);
            size_t parent_val = priorityFnPtr(data.data[parent_idx]);

            if (curr_val < parent_val)
            {
                T tmp = data.data[parent_idx];
                data.data[parent_idx] = data.data[idx];
                data.data[idx] = tmp;

                idx = parent_idx;
                continue;
            }

            break;
        }
    }
};