#pragma once

#include <Testing/BTest.h>
#include <Containers/Queue.h>
#include <Allocators/Allocator.h>

namespace Tests
{
    struct QueueTests
    {
        TEST_DECLARATION(Create)
        {
            CoreContext::DefaultContext();

            size_t size = 5;
            Queue<int> queue = {};

            Allocator allocator = HeapAllocator::Create();
            Queue<int>::Create(&queue, size , allocator);

            for (size_t i = 0; i < size; ++i)
            {
                int val = 69;
                Queue<int>::Enqueue(&queue , val + (int)i);
            }

            EVALUATE(queue.size == size);

            TEST_END()
        }

        TEST_DECLARATION(Enqueue)
        {
            CoreContext::DefaultContext();

            size_t size = 5;
            Queue<int> queue = {};

            Allocator allocator = HeapAllocator::Create();
            Queue<int>::Create(&queue, size , allocator);

            for (size_t i = 0; i < size; ++i)
            {
                int val = 69 + i;
                Queue<int>::Enqueue(&queue , val);
                
                EVALUATE(queue.size == (i + 1));
            
            }
            
            EVALUATE(queue.size == size);

            for (size_t i = 0; i < size; ++i)
            {
                int test = 69 + (int)i;
                int val = {};

                EVALUATE(queue.size == (size - i));

                Queue<int>::Dequeue(&queue , &val);

                EVALUATE(queue.size == (size - i - 1));
                
                EVALUATE(val == test)               
            }

            EVALUATE(queue.size == 0);

            TEST_END()
        }


        static inline DArray<TestCallback> GetAll() 
        {
            Allocator alloc = HeapAllocator::Create();
            DArray<TestCallback> arr = {};
            DArray<TestCallback>::Create(2 , &arr , alloc);

            DArray<TestCallback>::Add(&arr , QueueTests::Create);
            DArray<TestCallback>::Add(&arr , QueueTests::Enqueue);

            return arr;
        }; 
    };
}
