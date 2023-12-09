#include "pch.h"
#include "CppUnitTest.h"
#include <Containers/CArray.h>
#include <Allocators/Allocator.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace BEngineMathsUnitTests
{
    TEST_CLASS(CArrayTests)
    {
    public:

        TEST_METHOD(Create)
        {
            size_t size = 5;
            CArray<int> arr = {};

            Allocator allocator = HeapAllocator::Create();
            CArray<int>::Create(size, &arr, allocator);

            for (size_t i = 0; i < size; ++i)
            {
                int as_int = (int)i;
                CArray<int>::Add(&arr, as_int);
            }

            for (size_t i = 0; i < size; ++i)
            {
                Assert::IsTrue(arr.data[i] == i);
            }

            Assert::IsTrue(arr.capacity == size + 1);
            Assert::IsTrue(arr.size == size);
            Assert::IsTrue(arr.data != nullptr);

            //CArray<int>::Destroy(&&arr);
        }

        TEST_METHOD(Add)
        {
            CArray<int>* arr = {};

            Allocator allocator = HeapAllocator::Create();
            CArray<int>::Create(3, arr, allocator);

            Assert::IsTrue(arr->size == 0);
            Assert::IsTrue(arr->capacity == 4);

            CArray<int>::Add(arr, 6);

            Assert::IsTrue(arr->size == 1);
            Assert::IsTrue(arr->capacity == 4);

            CArray<int>::Add(arr, 9);

            Assert::IsTrue(arr->size == 2);
            Assert::IsTrue(arr->capacity == 4);

            CArray<int>::Add(arr, 4);

            Assert::IsTrue(arr->size == 3);
            Assert::IsTrue(arr->capacity == 4);


            Assert::IsTrue(arr->data[0] == 6);
            Assert::IsTrue(arr->data[1] == 9);
            Assert::IsTrue(arr->data[2] == 4);

            CArray<int>::Destroy(&arr);
        }

        TEST_METHOD(RemoveAll)
        {
            CArray<int>* arr = {};

            Allocator allocator = HeapAllocator::Create();
            CArray<int>::Create(5, arr, allocator);

            CArray<int>::Add(arr, 0);
            CArray<int>::Add(arr, 0);
            CArray<int>::Add(arr, 4);
            CArray<int>::Add(arr, 0);
            CArray<int>::Add(arr, 4);

            Allocator tmp_alloc = HeapAllocator::Create();
            size_t remove_count = CArray<int>::RemoveAll(arr, 4 , tmp_alloc);

            Assert::IsTrue(arr->data[0] == 0);
            Assert::IsTrue(arr->data[1] == 0);
            Assert::IsTrue(arr->data[2] == 0);
            Assert::IsTrue(arr->size == 3);
            Assert::IsTrue(remove_count == 2);

            CArray<int>::Destroy(&arr);
        }

        TEST_METHOD(TryIndexOf)
        {
            CArray<int>* arr = {};

            Allocator allocator = HeapAllocator::Create();
            CArray<int>::Create(5, arr, allocator);

            CArray<int>::Add(arr, 0);
            CArray<int>::Add(arr, 0);
            CArray<int>::Add(arr, 4);
            CArray<int>::Add(arr, 0);
            CArray<int>::Add(arr, 4);

            size_t index = 0;
            bool has_found = CArray<int>::TryIndexOf(arr, 0, arr->size, 4, &index);

            Assert::IsTrue(has_found);
            Assert::IsTrue(index == 2);
        }
    };
}
