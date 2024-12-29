#pragma once

#include "BTest.h"
#include <String/StringBuffer.h>
#include <String/StringView.h>
#include <String/StringUtils.h>
#include <Allocators/Allocator.h>

namespace Tests
{
    struct StringTests
    {
        TEST_DECLARATION(TestGetLength)
        TEST_BODY
        ({
            CoreContext::DefaultContext();

            StringView str1 = "One";
            StringView str2 = "Two";
            StringView str3 = "Three";

            size_t length = StringUtils::GetLength( str1, str2, str3 );

            EVALUATE( length == 3 + 3 + 5 );
        })


        TEST_DECLARATION(TestConcat)
        TEST_BODY
        ({
            CoreContext::DefaultContext();

            size_t total_length = StringUtils::GetLength( "One", "Two", "Three" );
            Allocator alloc = STACK_ALLOC( total_length );
            StringBuffer result = StringUtils::Concat( alloc, total_length, "One", "Two", "Three" );

            Allocator cStrPtr = STACK_ALLOC( total_length + 1 );
            const char* cStr = StringView::ToCString( result.view, cStrPtr );

            EVALUATE( result.length == 3 + 3 + 5 );
        })

        TEST_DECLARATION(TestFormat)
        TEST_BODY
        ({
            CoreContext::DefaultContext();

            Allocator alloc = ArenaAllocator::Create( &CoreContext::core_arena );

            StringBuffer result = StringUtils::Format( alloc, "Hey {} World {} The rest", "Hello", "!!!!" );

            char* cStr = StringView::ToCString( result.view, alloc );
            EVALUATE( strcmp( cStr, "Hey Hello World !!!! The rest" ) == 0 );
        })
        
        static inline DArray<TestCallback> GetAll() 
        {
            Allocator alloc = HeapAllocator::Create();
            DArray<TestCallback> arr = {};
            DArray<TestCallback>::Create(3 , &arr , alloc);

            DArray<TestCallback>::Add(&arr , StringTests::TestGetLength);
            DArray<TestCallback>::Add(&arr , StringTests::TestConcat);
            DArray<TestCallback>::Add(&arr , StringTests::TestFormat);

            return arr;
        };
    };
}
