#pragma once

#include <Testing/BTest.h>
#include <Containers/SlotArray.h>
#include <Allocators/Allocator.h>
#include <Containers/ContainerUtils.h>

namespace Tests
{
    struct SlotArrayTests
    {
        TEST_DECLARATION(Create)
        {
            CoreContext::DefaultContext();

            Allocator alloc = HeapAllocator::Create();
            SlotArray<float> arr = {};
            SlotArray<float>::Create(&arr , 5 , alloc);

            size_t idx_420 = arr.Add(420);
            EVALUATE(idx_420 == 0);
            EVALUATE(arr.size == 1);

            size_t idx_69 = arr.Add(69);
            EVALUATE(idx_69 == 1);
            EVALUATE(arr.size == 2);

            SlotArray<float>::Destroy(&arr);

            TEST_END()
        }

        TEST_DECLARATION(AddAndRemove)
        {
            CoreContext::DefaultContext();

            Allocator alloc = HeapAllocator::Create();
            SlotArray<float> arr = {};
            SlotArray<float>::Create(&arr , 5 , alloc);

            size_t idx_420 = arr.Add(420);
            EVALUATE(idx_420 == 0);
            EVALUATE(arr.HasIndex(0));
            EVALUATE(arr.size == 1);

            size_t idx_69 = arr.Add(69);
            EVALUATE(idx_69 == 1);
            EVALUATE(arr.HasIndex(1));
            EVALUATE(arr.size == 2);
            
            arr.RemoveAt(idx_420);
            EVALUATE(!arr.HasIndex(idx_420));
            EVALUATE(arr.HasIndex(idx_69));
            EVALUATE(arr.size == 1);
            
            SlotArray<float>::Destroy(&arr);

            TEST_END()
        }


        TEST_DECLARATION(GetAllWithIdx)
        {
            CoreContext::DefaultContext();

            Allocator alloc = HeapAllocator::Create();
            SlotArray<float> arr = {};
            SlotArray<float>::Create(&arr , 5 , alloc);

            size_t idx_420 = arr.Add(420);
            EVALUATE(idx_420 == 0);
            EVALUATE(arr.HasIndex(0));
            EVALUATE(arr.size == 1);

            size_t idx_69 = arr.Add(69);
            EVALUATE(idx_69 == 1);
            EVALUATE(arr.HasIndex(1));
            EVALUATE(arr.size == 2);
            
            arr.RemoveAt(idx_420);
            EVALUATE(!arr.HasIndex(idx_420));
            EVALUATE(arr.HasIndex(idx_69));
            EVALUATE(arr.size == 1);
            
            size_t idx_999 = arr.Add(999);
            EVALUATE(idx_999 == 0);
            EVALUATE(arr.HasIndex(idx_999));
            EVALUATE(arr.size == 2);
            
            DArray<SlotArray<float>::SlotArrayElem> all = {};
            DArray<SlotArray<float>::SlotArrayElem>::Create(1 , &all , alloc);

            arr.GetAll(&all);

            EVALUATE(all.size == arr.size);
            EVALUATE(all.size == 2)

            EVALUATE(all.data[0].idx == 0);
            EVALUATE(all.data[0].item == 999);

            EVALUATE(all.data[1].idx == 1);
            EVALUATE(all.data[1].item == 69);

            DArray<SlotArray<float>::SlotArrayElem>::Destroy(&all);
            SlotArray<float>::Destroy(&arr);

            TEST_END()
        }

        TEST_DECLARATION(Destroy)
        {
            CoreContext::DefaultContext();

            Allocator alloc = HeapAllocator::Create();
            SlotArray<float> arr = {};
            SlotArray<float>::Create(&arr , 5 , alloc);

            size_t idx_420 = arr.Add(420);
            size_t idx_69 = arr.Add(69);

            EVALUATE(arr.size == 2);
            SlotArray<float>::Destroy(&arr);

            SlotArray<float> empty_arr = {};
            bool is_same = CoreContext::mem_compare(&arr , &empty_arr , sizeof(SlotArray<float>));

            EVALUATE(is_same);

            TEST_END()
        }

        static inline DArray<TestCallback> GetAll() 
        {
            Allocator alloc = HeapAllocator::Create();
            DArray<TestCallback> arr = {};
            DArray<TestCallback>::Create(4 , &arr , alloc);

            DArray<TestCallback>::Add(&arr , SlotArrayTests::Create);
            DArray<TestCallback>::Add(&arr , SlotArrayTests::AddAndRemove);
            DArray<TestCallback>::Add(&arr , SlotArrayTests::GetAllWithIdx);
            DArray<TestCallback>::Add(&arr , SlotArrayTests::Destroy);

            return arr;
        }; 
    };
}
