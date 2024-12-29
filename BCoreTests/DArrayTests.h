#pragma once

#include <Testing/BTest.h>
#include <Containers/DArray.h>
#include <Allocators/Allocator.h>

namespace Tests
{
    struct DArrayTests
    {
        TEST_DECLARATION(Create)
        TEST_BODY
        ({
            CoreContext::DefaultContext();

            size_t size = 5;
            DArray<int> arr;

            Allocator allocator = HeapAllocator::Create();
            DArray<int>::Create(size, &arr, allocator);

            for (size_t i = 0; i < size; ++i)
            {
                int as_int = (int)i;
                DArray<int>::Add(&arr, as_int);
            }

            for (size_t i = 0; i < size; ++i)
            {
                EVALUATE(arr.data[i] == i);
            }

            EVALUATE(arr.capacity == size);
            EVALUATE(arr.size == size);
            EVALUATE(arr.data != nullptr);
        })

        TEST_DECLARATION(Add)
        TEST_BODY
        ({
            CoreContext::DefaultContext();

            DArray<int> arr;

            Allocator allocator = HeapAllocator::Create();
            DArray<int>::Create(0, &arr, allocator);


            EVALUATE(arr.size == 0);
            EVALUATE(arr.capacity == 0);

            DArray<int>::Add(&arr, 6);

            EVALUATE(arr.size == 1);
            EVALUATE(arr.capacity == 1);

            DArray<int>::Add(&arr, 9);

            EVALUATE(arr.size == 2);
            EVALUATE(arr.capacity == 3);
            
            DArray<int>::Add(&arr, 4);

            EVALUATE(arr.size == 3);
            EVALUATE(arr.capacity == 3);

            EVALUATE(arr.data[0] == 6);
            EVALUATE(arr.data[1] == 9);
            EVALUATE(arr.data[2] == 4);
        })

        TEST_DECLARATION(RemoveAll)
        TEST_BODY
        ({
            CoreContext::DefaultContext();

            DArray<int> arr;

            Allocator allocator = HeapAllocator::Create();
            DArray<int>::Create(0, &arr, allocator);

            DArray<int>::Add(&arr, 0);
            DArray<int>::Add(&arr, 0);
            DArray<int>::Add(&arr, 4);
            DArray<int>::Add(&arr, 0);
            DArray<int>::Add(&arr, 4);

            size_t remove_count = DArray<int>::RemoveAll(&arr, 4);

            EVALUATE(arr.data[0] == 0);
            EVALUATE(arr.data[1] == 0);
            EVALUATE(arr.data[2] == 0);
            EVALUATE(arr.size == 3);
            EVALUATE(remove_count == 2);
        })

        TEST_DECLARATION(TryIndexOf)
        TEST_BODY
        ({
            CoreContext::DefaultContext();

            DArray<int> arr;

            Allocator allocator = HeapAllocator::Create();
            DArray<int>::Create(0, &arr, allocator);

            DArray<int>::Add(&arr, 0);
            DArray<int>::Add(&arr, 0);
            DArray<int>::Add(&arr, 4);
            DArray<int>::Add(&arr, 0);
            DArray<int>::Add(&arr, 4);

            size_t index = 0;
            bool has_found = DArray<int>::TryIndexOf(&arr, 0 , arr.size - 1 , 4 , &index);

            EVALUATE(has_found);
            EVALUATE(index == 2);
        })

        static inline DArray<TestCallback> GetAll() 
        {
            Allocator alloc = HeapAllocator::Create();
            DArray<TestCallback> arr = {};
            DArray<TestCallback>::Create(4 , &arr , alloc);

            DArray<TestCallback>::Add(&arr , DArrayTests::Create);
            DArray<TestCallback>::Add(&arr , DArrayTests::Add);
            DArray<TestCallback>::Add(&arr , DArrayTests::RemoveAll);
            DArray<TestCallback>::Add(&arr , DArrayTests::TryIndexOf);

            return arr;
        }; 
    };
}
