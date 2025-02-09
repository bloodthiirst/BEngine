#pragma once

#include <Testing/BTest.h>
#include <Containers/DArray.h>
#include <Allocators/Allocator.h>

namespace Tests
{
    struct BlankTests
    {
        TEST_DECLARATION(Test)
        {
            CoreContext::DefaultContext();

            EVALUATE(true);
            
            TEST_END()
        }

        static inline DArray<TestCallback> GetAll() 
        {
            Allocator alloc = HeapAllocator::Create();
            DArray<TestCallback> arr = {};
            DArray<TestCallback>::Create(4 , &arr , alloc);

            DArray<TestCallback>::Add(&arr , BlankTests::Test);

            return arr;
        }; 
    };
}
