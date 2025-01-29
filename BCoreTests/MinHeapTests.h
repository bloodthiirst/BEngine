#pragma once

#include <Testing/BTest.h>
#include <Containers/MinHeap.h>
#include <Allocators/Allocator.h>

namespace Tests
{
    struct MinHeapTests
    {
        struct Box
        {
            size_t priority;
            size_t value;
        };
        
        static size_t BoxToPriority(Box b)
        {
            return b.priority;
        }

        TEST_DECLARATION(Create)
        {
            CoreContext::DefaultContext();

            size_t size = 5;
            MinHeap<Box> heap = {};

            Allocator allocator = HeapAllocator::Create();
            MinHeap<Box>::Create(allocator, &heap , size , BoxToPriority);

            for (size_t i = 0; i < size; ++i)
            {
                size_t val = 69;

                Box b = {};
                b.priority = size - i - 1;
                b.value = val;

                heap.Add(b);
            }

            EVALUATE(heap.data.data != nullptr);
            EVALUATE(heap.data.size == size);

            TEST_END()
        }

        TEST_DECLARATION(Add)
        {
            CoreContext::DefaultContext();

            size_t size = 5;
            MinHeap<Box> heap = {};

            Allocator allocator = HeapAllocator::Create();
            MinHeap<Box>::Create(allocator, &heap , size , BoxToPriority);

            for (size_t i = 0; i < size; ++i)
            {
                size_t val = 69;

                Box b = {};
                b.priority = size - i - 1;
                b.value = val;

                heap.Add(b);

                Box peek = heap.Peek();
                EVALUATE( CoreContext::mem_compare(&peek , &b , sizeof(Box) ));
            }

            EVALUATE(heap.data.data != nullptr);
            EVALUATE(heap.data.size == size);

            TEST_END()
        }


        static inline DArray<TestCallback> GetAll() 
        {
            Allocator alloc = HeapAllocator::Create();
            DArray<TestCallback> arr = {};
            DArray<TestCallback>::Create(2 , &arr , alloc);

            DArray<TestCallback>::Add(&arr , MinHeapTests::Create);
            DArray<TestCallback>::Add(&arr , MinHeapTests::Add);

            return arr;
        }; 
    };
}
