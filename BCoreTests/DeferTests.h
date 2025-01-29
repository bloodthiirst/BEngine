#pragma once

#include <Testing/BTest.h>
#include <Containers/DArray.h>
#include <Allocators/Allocator.h>
#include <Defer/Defer.h>

namespace Tests
{
    struct DeferTests
    {
        TEST_DECLARATION(Create)
        {
            int val = 1;

            {
                DEFER([&](){ val++; });
            }

            EVALUATE(val == 2);            
            TEST_END()
        }

        static inline DArray<TestCallback> GetAll() 
        {
            Allocator alloc = HeapAllocator::Create();
            DArray<TestCallback> arr = {};
            DArray<TestCallback>::Create(1 , &arr , alloc);

            DArray<TestCallback>::Add(&arr , DeferTests::Create);

            return arr;
        }; 
    };
}
