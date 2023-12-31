#include "pch.h"
#include "CppUnitTest.h"
#include <Containers/DArray.h>
#include <Allocators/Allocator.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace BEngineMathsUnitTests
{
    TEST_CLASS(DArrayTests)
    {
    public:

        TEST_METHOD(Create)
        {
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
                Assert::IsTrue(arr.data[i] == i);
            }

            Assert::IsTrue(arr.capacity == size);
            Assert::IsTrue(arr.size == size);
            Assert::IsTrue(arr.data != nullptr);
        }

        TEST_METHOD(Add)
        {
            CoreContext::DefaultContext();

            DArray<int> arr;

            Allocator allocator = HeapAllocator::Create();
            DArray<int>::Create(0, &arr, allocator);


            Assert::IsTrue(arr.size == 0);
            Assert::IsTrue(arr.capacity == 0);

            DArray<int>::Add(&arr, 6);

            Assert::IsTrue(arr.size == 1);
            Assert::IsTrue(arr.capacity == 1);

            DArray<int>::Add(&arr, 9);

            Assert::IsTrue(arr.size == 2);
            Assert::IsTrue(arr.capacity == 3);
            
            DArray<int>::Add(&arr, 4);

            Assert::IsTrue(arr.size == 3);
            Assert::IsTrue(arr.capacity == 3);


            Assert::IsTrue(arr.data[0] == 6);
            Assert::IsTrue(arr.data[1] == 9);
            Assert::IsTrue(arr.data[2] == 4);
        }

        TEST_METHOD(RemoveAll)
        {

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

            Assert::IsTrue(arr.data[0] == 0);
            Assert::IsTrue(arr.data[1] == 0);
            Assert::IsTrue(arr.data[2] == 0);
            Assert::IsTrue(arr.size == 3);
            Assert::IsTrue(remove_count == 2);
        }

        TEST_METHOD(TryIndexOf)
        {
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
            bool has_found = DArray<int>::TryIndexOf(&arr, 0 , arr.size , 4 , &index);

            Assert::IsTrue(has_found);
            Assert::IsTrue(index == 2);
        }
    };
}
