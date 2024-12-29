#pragma once

#include "BTest.h"
#include <Containers/DArray.h>
#include <Containers/HMap.h>
#include <Allocators/Allocator.h>

namespace Tests
{
    size_t IntHasher( int val )
    {
        return (size_t) val;
    }

    bool IntComparer( int a, int b )
    {
        return a == b;
    }

    struct HMapTests
    {
        TEST_DECLARATION(Create)
        TEST_BODY
        ({
            CoreContext::DefaultContext();

            size_t capacity = 10;
            size_t size = 5;

            HMap<int, float> map = {};

            Allocator allocator = HeapAllocator::Create();
            HMap<int, float>::Create( &map, allocator, capacity, 5, IntHasher, IntComparer );


            for ( size_t i = 0; i < size; ++i )
            {
                int as_int = (int) i;
                size_t out_idx = {};
                HMap<int, float>::TryAdd( &map, as_int, 420.0f + i, &out_idx );
            }

            DArray<Pair<int, float>> result = {};
            DArray<Pair<int, float>>::Create( map.count, &result, allocator );

            HMap<int, float>::GetAll( &map, &result );

            EVALUATE( map.capacity == capacity );
            EVALUATE( map.count == size );
            EVALUATE( map.all_keys.size == size );
            EVALUATE( map.all_values.size == size );
        })

        TEST_DECLARATION(TryGet)
        TEST_BODY
        ({
            CoreContext::DefaultContext();

            size_t capacity = 10;
            size_t size = 5;

            HMap<int, float> map = {};

            Allocator allocator = HeapAllocator::Create();
            HMap<int, float>::Create( &map, allocator, capacity, 5, IntHasher, IntComparer );


            for ( size_t i = 0; i < size; ++i )
            {
                int as_int = (int) i;
                size_t out_idx = {};
                HMap<int, float>::TryAdd( &map, (int) i, 420.0f + i, &out_idx );
            }

            float* zero_value = {};
            bool has_zero_key = HMap<int, float>::TryGet( &map, 0, &zero_value );

            float* six_value = {};
            bool has_six_key = HMap<int, float>::TryGet( &map, 6, &six_value );

            EVALUATE( has_zero_key );
            EVALUATE( !has_six_key );

            EVALUATE( zero_value != 0 );
            EVALUATE( six_value == 0 );

            EVALUATE( *zero_value == 420 );
        })


        TEST_DECLARATION(TryAdd)
        TEST_BODY
        ({
            CoreContext::DefaultContext();

            size_t capacity = 10;
            size_t size = 5;

            HMap<int, float> map = {};

            Allocator allocator = HeapAllocator::Create();
            HMap<int, float>::Create( &map, allocator, capacity, 5, IntHasher, IntComparer );


            for ( size_t i = 0; i < size; ++i )
            {
                int as_int = (int) i;
                size_t out_idx = {};
                HMap<int, float>::TryAdd( &map, 0, 420.0f + i, &out_idx );
            }

            DArray<Pair<int, float>> result = {};
            DArray<Pair<int, float>>::Create( map.count, &result, allocator );

            HMap<int, float>::GetAll( &map, &result );

            EVALUATE( map.capacity == capacity );
            EVALUATE( map.count == 1 );
            EVALUATE( map.all_keys.size == 1 );
            EVALUATE( map.all_values.size == 1 );
        })

        TEST_DECLARATION(TryRemove)
        TEST_BODY
        ({
            CoreContext::DefaultContext();

            size_t capacity = 10;
            size_t size = 5;

            HMap<int, float> map = {};

            Allocator allocator = HeapAllocator::Create();
            HMap<int, float>::Create( &map, allocator, capacity, 5, IntHasher, IntComparer );


            for ( int i = 0; i < size; ++i )
            {
                size_t out_idx = {};
                HMap<int, float>::TryAdd( &map,i, 420.0f + i, &out_idx );
            }

            float fifty_removed = {};
            float three_removed = {};
            HMap<int, float>::TryRemove( &map, 50, &fifty_removed );
            HMap<int, float>::TryRemove( &map, 3, &three_removed );

            DArray<Pair<int, float>> result = {};
            DArray<Pair<int, float>>::Create( map.count, &result, allocator );

            HMap<int, float>::GetAll( &map, &result );

            EVALUATE( map.capacity == capacity );
            EVALUATE( map.count == 4 );
            EVALUATE( map.all_keys.size == 4 );
            EVALUATE( map.all_values.size == 4 );
        })

        static inline DArray<TestCallback> GetAll() 
        {
            Allocator alloc = HeapAllocator::Create();
            DArray<TestCallback> arr = {};
            DArray<TestCallback>::Create(4 , &arr , alloc);

            DArray<TestCallback>::Add(&arr , HMapTests::Create);
            DArray<TestCallback>::Add(&arr , HMapTests::TryGet);
            DArray<TestCallback>::Add(&arr , HMapTests::TryAdd);
            DArray<TestCallback>::Add(&arr , HMapTests::TryRemove);

            return arr;
        };
    };
}
