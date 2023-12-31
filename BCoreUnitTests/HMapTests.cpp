#include "pch.h"
#include "CppUnitTest.h"
#include <Containers/DArray.h>
#include <Containers/HMap.h>
#include <Allocators/Allocator.h>
#include <iostream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace BEngineMathsUnitTests
{
    size_t IntHasher( int val )
    {
        return (size_t) val;
    }

    bool IntComparer( int a, int b )
    {
        return a == b;
    }


    TEST_CLASS( HMapTests )
    {
    public:

        TEST_METHOD( Create )
        {
            CoreContext::DefaultContext();

            size_t capacity = 10;
            size_t size = 5;

            HMap<int, float> map = {};

            Allocator allocator = HeapAllocator::Create();
            HMap<int, float>::Create( &map, allocator, capacity, 5, IntHasher, IntComparer );


            for ( size_t i = 0; i < size; ++i )
            {
                int as_int = (int) i;
                HMap<int, float>::TryAdd( &map, as_int, 420.0f + i );
            }

            DArray<Pair<int, float>> result = {};
            DArray<Pair<int, float>>::Create( map.count, &result, allocator );

            HMap<int, float>::GetAll( &map, &result );

            for ( size_t i = 0; i < result.size; ++i )
            {
                Pair<int, float> p = result.data[i];
                std::cout << p.key << "," << p.value << '\n';
            }

            Assert::IsTrue( map.capacity == capacity );
            Assert::IsTrue( map.count == size );
            Assert::IsTrue( map.all_keys.size == size );
            Assert::IsTrue( map.all_values.size == size );
        }

        TEST_METHOD( TryGet )
        {
            CoreContext::DefaultContext();

            size_t capacity = 10;
            size_t size = 5;

            HMap<int, float> map = {};

            Allocator allocator = HeapAllocator::Create();
            HMap<int, float>::Create( &map, allocator, capacity, 5, IntHasher, IntComparer );


            for ( size_t i = 0; i < size; ++i )
            {
                int as_int = (int) i;
                HMap<int, float>::TryAdd( &map, i, 420.0f + i );
            }

            float* zero_value = {};
            bool has_zero_key = HMap<int, float>::TryGet( &map, 0, &zero_value );

            float* six_value = {};
            bool has_six_key = HMap<int, float>::TryGet( &map, 6, &six_value );

            Assert::IsTrue( has_zero_key );
            Assert::IsFalse( has_six_key );

            Assert::IsTrue( zero_value != 0 );
            Assert::IsTrue( six_value == 0 );

            Assert::IsTrue( *zero_value == 420 );
        }
        TEST_METHOD( TryAdd )
        {
            CoreContext::DefaultContext();

            size_t capacity = 10;
            size_t size = 5;

            HMap<int, float> map = {};

            Allocator allocator = HeapAllocator::Create();
            HMap<int, float>::Create( &map, allocator, capacity, 5, IntHasher, IntComparer );


            for ( size_t i = 0; i < size; ++i )
            {
                int as_int = (int) i;
                HMap<int, float>::TryAdd( &map, 0, 420.0f + i );
            }

            DArray<Pair<int, float>> result = {};
            DArray<Pair<int, float>>::Create( map.count, &result, allocator );

            HMap<int, float>::GetAll( &map, &result );

            for ( size_t i = 0; i < result.size; ++i )
            {
                Pair<int, float> p = result.data[i];
                std::cout << p.key << "," << p.value << '\n';
            }

            Assert::IsTrue( map.capacity == capacity );
            Assert::IsTrue( map.count == 1 );
            Assert::IsTrue( map.all_keys.size == 1 );
            Assert::IsTrue( map.all_values.size == 1 );
        }

        TEST_METHOD( TryRemove )
        {
            CoreContext::DefaultContext();

            size_t capacity = 10;
            size_t size = 5;

            HMap<int, float> map = {};

            Allocator allocator = HeapAllocator::Create();
            HMap<int, float>::Create( &map, allocator, capacity, 5, IntHasher, IntComparer );


            for ( size_t i = 0; i < size; ++i )
            {
                int as_int = (int) i;
                HMap<int, float>::TryAdd( &map, i, 420.0f + i );
            }

            float fifty_removed = {};
            float three_removed = {};
            HMap<int, float>::TryRemove( &map, 50, &fifty_removed );
            HMap<int, float>::TryRemove( &map, 3 ,&three_removed );

            DArray<Pair<int, float>> result = {};
            DArray<Pair<int, float>>::Create( map.count, &result, allocator );

            HMap<int, float>::GetAll( &map, &result );

            for ( size_t i = 0; i < result.size; ++i )
            {
                Pair<int, float> p = result.data[i];
                std::cout << p.key << "," << p.value << '\n';
            }

            Assert::IsTrue( map.capacity == capacity );
            Assert::IsTrue( map.count == 4 );
            Assert::IsTrue( map.all_keys.size == 4 );
            Assert::IsTrue( map.all_values.size == 4 );
        }
    };
}
