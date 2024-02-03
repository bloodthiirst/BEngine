#include "pch.h"
#include "CppUnitTest.h"
#include <Allocators/Allocator.h>
#include <Containers/ContainerUtils.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BEngineMathsUnitTests
{
    TEST_CLASS( ContainerUtilsTests )
    {
    public:

        static bool Sort( size_t a, size_t b )
        {
            return a < b;
        }

        TEST_METHOD( SortTest )
        {
            CoreContext::DefaultContext();

            size_t size = 5;

            Allocator allocator = HeapAllocator::Create();
            size_t* nums = (size_t*) allocator.alloc( allocator, sizeof( size_t ) * size );

            for ( size_t i = 0; i < size; ++i )
            {
                nums[i] = i;
            }

            ContainerUtils::Sort( nums, 0, size, Sort );

            for ( size_t i = 0; i < size; ++i )
            {
                Assert::IsTrue( nums[i] == (size - 1 - i));
            }
        }
    };
}
