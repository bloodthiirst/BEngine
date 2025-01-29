#pragma once

#include <Testing/BTest.h>
#include <Allocators/Allocator.h>
#include <Containers/ContainerUtils.h>

namespace Tests
{
    struct ContainerUtilsTests
    {
        static bool Sort(size_t a, size_t b)
        {
            return a < b;
        }

        TEST_DECLARATION(SortTest)
        {
            size_t size = 5;

            Allocator allocator = HeapAllocator::Create();
            size_t *nums = (size_t *)allocator.alloc(&allocator, sizeof(size_t) * size);

            for (size_t i = 0; i < size; ++i)
            {
                nums[i] = i;
            }

            ContainerUtils::Sort(nums, 0, size, Sort);

            for (size_t i = 0; i < size; ++i)
            {
                EVALUATE(nums[i] == (size - 1 - i));
            }

            TEST_END()
        }

        static inline DArray<TestCallback> GetAll() 
        {
            Allocator alloc = HeapAllocator::Create();
            DArray<TestCallback> arr = {};
            DArray<TestCallback>::Create(4 , &arr , alloc);

            DArray<TestCallback>::Add(&arr , ContainerUtilsTests::SortTest);

            return arr;
        }; 
    };
}
