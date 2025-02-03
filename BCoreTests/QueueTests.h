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
                int val = 69 + (int)i;
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

        TEST_DECLARATION(EnqueueThenDequeue)
        {
            CoreContext::DefaultContext();

            size_t size = 5;
            Queue<int> queue = {};

            Allocator allocator = HeapAllocator::Create();
            Queue<int>::Create(&queue, size , allocator);
            
            EVALUATE(queue.size == 0);            
            EVALUATE(queue.enqueue_index == 0);
            EVALUATE(queue.dequeque_index == 0);

            Queue<int>::Enqueue(&queue , 1); // 0
            EVALUATE(queue.size == 1);
            EVALUATE(queue.enqueue_index == 1);
            EVALUATE(queue.dequeque_index == 0);

            Queue<int>::Enqueue(&queue , 1); // 1
            EVALUATE(queue.size == 2);
            EVALUATE(queue.enqueue_index == 2);
            EVALUATE(queue.dequeque_index == 0);

            Queue<int>::Enqueue(&queue , 1); // 2      
            EVALUATE(queue.size == 3);
            EVALUATE(queue.enqueue_index == 3);
            EVALUATE(queue.dequeque_index == 0);

            int val = {};
            Queue<int>::Dequeue(&queue , &val); // 0
            EVALUATE(queue.size == 2);
            EVALUATE(queue.enqueue_index == 3);
            EVALUATE(queue.dequeque_index == 1);

            Queue<int>::Dequeue(&queue , &val); // 1
            EVALUATE(queue.size == 1);
            EVALUATE(queue.enqueue_index == 3);
            EVALUATE(queue.dequeque_index == 2);
            
            Queue<int>::Enqueue(&queue , 1); // 3
            EVALUATE(queue.size == 2);
            EVALUATE(queue.enqueue_index == 4);
            EVALUATE(queue.dequeque_index == 2);

            Queue<int>::Enqueue(&queue , 1); // 4
            EVALUATE(queue.size == 3);
            EVALUATE(queue.enqueue_index == 0);
            EVALUATE(queue.dequeque_index == 2);
            
            Queue<int>::Enqueue(&queue , 1); // 0
            EVALUATE(queue.size == 4);            
            EVALUATE(queue.enqueue_index == 1);
            EVALUATE(queue.dequeque_index == 2);

            Queue<int>::Enqueue(&queue , 1); // 1
            EVALUATE(queue.size == 5);            
            EVALUATE(queue.enqueue_index == 2);
            EVALUATE(queue.dequeque_index == 2);

            Queue<int>::Dequeue(&queue , &val); // 1
            EVALUATE(queue.size == 4);            
            EVALUATE(queue.enqueue_index == 2);
            EVALUATE(queue.dequeque_index == 3);

            Queue<int>::Dequeue(&queue , &val); // 1
            EVALUATE(queue.size == 3);            
            EVALUATE(queue.enqueue_index == 2);
            EVALUATE(queue.dequeque_index == 4);

            Queue<int>::Dequeue(&queue , &val); // 1
            EVALUATE(queue.size == 2);            
            EVALUATE(queue.enqueue_index == 2);
            EVALUATE(queue.dequeque_index == 0);

            TEST_END()
        }
        
        static inline DArray<TestCallback> GetAll() 
        {
            Allocator alloc = HeapAllocator::Create();
            DArray<TestCallback> arr = {};
            DArray<TestCallback>::Create(2 , &arr , alloc);

            DArray<TestCallback>::Add(&arr , QueueTests::Create);
            DArray<TestCallback>::Add(&arr , QueueTests::Enqueue);
            DArray<TestCallback>::Add(&arr , QueueTests::EnqueueThenDequeue);

            return arr;
        }; 
    };
}
