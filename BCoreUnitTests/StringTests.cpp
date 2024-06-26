#include "pch.h"
#include "CppUnitTest.h"
#include <String/StringBuffer.h>
#include <String/StringView.h>
#include <String/StringUtils.h>
#include <Allocators/Allocator.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace BEngineMathsUnitTests
{
    TEST_CLASS( StringTests )
    {
    public:

        TEST_METHOD( TestGetLength )
        {
            CoreContext::DefaultContext();

            StringView str1 = "One";
            StringView str2 = "Two";
            StringView str3 = "Three";

            size_t length = StringUtils::GetLength( str1, str2, str3 );

            Assert::IsTrue( length == 3 + 3 + 5 );
        }

        TEST_METHOD( TestConcat )
        {
            CoreContext::DefaultContext();

            size_t total_length = StringUtils::GetLength( "One", "Two", "Three" );
            Allocator alloc = STACK_ALLOC( total_length );
            StringBuffer result = StringUtils::Concat( alloc, total_length, "One", "Two", "Three" );

            Allocator cStrPtr = STACK_ALLOC( total_length + 1 );
            const char* cStr = StringView::ToCString( result.view, cStrPtr );

            Assert::IsTrue( result.length == 3 + 3 + 5 );
        }

        TEST_METHOD( TestFormat )
        {
            CoreContext::DefaultContext();

            Allocator alloc = ArenaAllocator::Create( &CoreContext::core_arena );

            StringBuffer result = StringUtils::Format( alloc, "Hey {} World {} The rest", "Hello", "!!!!" );

            char* cStr = StringView::ToCString( result.view, alloc );
            Assert::IsTrue( strcmp( cStr, "Hey Hello World !!!! The rest" ) == 0 );
        }
    };
}
