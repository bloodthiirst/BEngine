#include "pch.h"
#include "CppUnitTest.h"
#include <Containers/FreeList.h>
#include <Allocators/Allocator.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace BEngineMathsUnitTests
{
    TEST_CLASS( FreeListTests )
    {
    public:

        TEST_METHOD( Create )
        {
            CoreContext::DefaultContext();

            size_t node_cap = 5;
            size_t mem = 64;
            FreeList flist;

            Allocator allocator = HeapAllocator::Create();
            FreeList::Create( &flist, node_cap, mem, allocator );

            Assert::IsTrue( flist.total_mem == mem );
            Assert::IsTrue( flist.used_mem == 0 );

            FreeList::Destroy( &flist );
        }

        TEST_METHOD( AllocBlock )
        {
            CoreContext::DefaultContext();

            size_t node_cap = 5;
            size_t total_mem = 64;
            FreeList flist;

            Allocator allocator = HeapAllocator::Create();
            FreeList::Create( &flist, node_cap, total_mem, allocator );

            size_t claim_size = 32;
            FreeList::Node node = {};
            FreeList::AllocBlock( &flist, claim_size, &node );

            Assert::IsTrue( node.size == claim_size );
            Assert::IsTrue( node.start == 0 );

            Assert::IsTrue( flist.total_mem == total_mem );
            Assert::IsTrue( flist.used_mem == claim_size );
            Assert::IsTrue( flist.used_nodes.size == 1 );
            Assert::IsTrue( flist.free_nodes.size == 1 );

            FreeList::Destroy( &flist );
        }

        TEST_METHOD( MultipleAllocBlock )
        {
            CoreContext::DefaultContext();

            size_t node_cap = 5;
            size_t total_mem = 512;
            FreeList flist;

            Allocator allocator = HeapAllocator::Create();
            FreeList::Create( &flist, node_cap, total_mem, allocator );


            const size_t claim_size = 32;
            const size_t alloc_count = 16;
            FreeList::Node node[alloc_count] = {};

            for ( size_t i = 0; i < alloc_count; ++i )
            {
                FreeList::AllocBlock( &flist, claim_size, &node[i] );
            }

            Assert::IsTrue( flist.total_mem == total_mem );
            Assert::IsTrue( flist.used_mem == claim_size * alloc_count );
            Assert::IsTrue( flist.used_nodes.size == alloc_count );
            Assert::IsTrue( flist.free_nodes.size == 0 );

            FreeList::Destroy( &flist );
        }

        TEST_METHOD( MultipleAllocBlockAndAFree )
        {
            CoreContext::DefaultContext();

            size_t node_cap = 5;
            size_t total_mem = 512;
            FreeList flist;

            Allocator allocator = HeapAllocator::Create();
            FreeList::Create( &flist, node_cap, total_mem, allocator );

            const size_t claim_size = 32;
            const size_t alloc_count = 16;
            FreeList::Node node[alloc_count] = {};

            for ( size_t i = 0; i < alloc_count; ++i )
            {
                FreeList::AllocBlock( &flist, claim_size, &node[i] );
            }

            FreeList::FreeBlock( &flist, FreeList::Node{ 32 ,32 } );
            FreeList::FreeBlock( &flist, FreeList::Node{ 0 , 32 } );

            Assert::IsTrue( flist.total_mem == total_mem );
            Assert::IsTrue( flist.used_mem == claim_size * (alloc_count - 2) );
            Assert::IsTrue( flist.used_nodes.size == (alloc_count - 2) );
            Assert::IsTrue( flist.free_nodes.size == 1 );

            FreeList::Destroy( &flist );
        }

        TEST_METHOD( MultipleAllocBlockAndAFreeOrdered )
        {
            CoreContext::DefaultContext();

            size_t node_cap = 5;
            size_t total_mem = 512;
            FreeList flist;

            Allocator allocator = HeapAllocator::Create();
            FreeList::Create( &flist, node_cap, total_mem, allocator );

            const size_t claim_size = 32;
            const size_t alloc_count = 16;
            FreeList::Node node[alloc_count] = {};

            for ( size_t i = 0; i < alloc_count; ++i )
            {
                FreeList::AllocBlock( &flist, claim_size, &node[i] );
            }

            FreeList::FreeBlock( &flist, FreeList::Node{ 32 ,32 } );
            FreeList::FreeBlock( &flist, FreeList::Node{ 64 ,32 } );

            Assert::IsTrue( flist.total_mem == total_mem );
            Assert::IsTrue( flist.used_mem == claim_size * (alloc_count - 2) );
            Assert::IsTrue( flist.used_nodes.size == (alloc_count - 2) );
            Assert::IsTrue( flist.free_nodes.size == 1 );

            FreeList::Destroy( &flist );
        }

        TEST_METHOD( FreeBlockThatNeedMerging )
        {
            CoreContext::DefaultContext();

            size_t node_cap = 5;
            size_t total_mem = 512;
            FreeList flist;

            Allocator allocator = HeapAllocator::Create();
            FreeList::Create( &flist, node_cap, total_mem, allocator );

            const size_t claim_size = 32;
            const size_t alloc_count = 16;
            FreeList::Node node[alloc_count] = {};

            for ( size_t i = 0; i < alloc_count; ++i )
            {
                FreeList::AllocBlock( &flist, claim_size, &node[i] );
            }

            FreeList::FreeBlock( &flist, FreeList::Node{ 32 ,32 } );
            FreeList::FreeBlock( &flist, FreeList::Node{ 128 ,32 } );
            FreeList::FreeBlock( &flist, FreeList::Node{ 64 ,32 } );

            Assert::IsTrue( flist.total_mem == total_mem );
            Assert::IsTrue( flist.used_mem == claim_size * (alloc_count - 3) );
            Assert::IsTrue( flist.used_nodes.size == (alloc_count - 3) );
            Assert::IsTrue( flist.free_nodes.size == 2 );

            FreeList::Destroy( &flist );
        }


        TEST_METHOD( FreeBlockThatDoesntNeedMerging )
        {
            CoreContext::DefaultContext();

            size_t node_cap = 5;
            size_t total_mem = 512;
            FreeList flist;

            Allocator allocator = HeapAllocator::Create();
            FreeList::Create( &flist, node_cap, total_mem, allocator );

            const size_t claim_size = 32;
            const size_t alloc_count = 16;
            FreeList::Node node[alloc_count] = {};

            for ( size_t i = 0; i < alloc_count; ++i )
            {
                FreeList::AllocBlock( &flist, claim_size, &node[i] );
            }

            FreeList::FreeBlock( &flist, FreeList::Node{ 64 ,32 } );
            FreeList::FreeBlock( &flist, FreeList::Node{ 128 ,32 } );

            Assert::IsTrue( flist.total_mem == total_mem );
            Assert::IsTrue( flist.used_mem == claim_size * (alloc_count - 2) );
            Assert::IsTrue( flist.used_nodes.size == (alloc_count - 2) );
            Assert::IsTrue( flist.free_nodes.size == 2 );

            FreeList::Destroy( &flist );
        }

        TEST_METHOD( MultipleAllocBlockAndAFreeReverseOrder )
        {
            CoreContext::DefaultContext();

            size_t node_cap = 5;
            size_t total_mem = 512;
            FreeList flist;

            Allocator allocator = HeapAllocator::Create();
            FreeList::Create( &flist, node_cap, total_mem, allocator );

            const size_t claim_size = 32;
            const size_t alloc_count = 16;
            FreeList::Node node[alloc_count] = {};

            for ( size_t i = 0; i < alloc_count; ++i )
            {
                FreeList::AllocBlock( &flist, claim_size, &node[i] );
            }

            FreeList::FreeBlock( &flist, FreeList::Node{ 64 ,32 } );
            FreeList::FreeBlock( &flist, FreeList::Node{ 32 ,32 } );

            Assert::IsTrue( flist.total_mem == total_mem );
            Assert::IsTrue( flist.used_mem == claim_size * (alloc_count - 2) );
            Assert::IsTrue( flist.used_nodes.size == (alloc_count - 2) );
            Assert::IsTrue( flist.free_nodes.size == 1 );

            FreeList::Destroy( &flist );
        }

        TEST_METHOD( MultipleAllocAllThenFreeAllInReverse )
        {
            CoreContext::DefaultContext();

            size_t node_cap = 5;
            size_t total_mem = 512;
            FreeList flist;

            Allocator allocator = HeapAllocator::Create();
            FreeList::Create( &flist, node_cap, total_mem, allocator );

            const size_t claim_size = 32;
            const size_t alloc_count = 16;
            FreeList::Node node[alloc_count] = {};

            for ( size_t i = 0; i < alloc_count; ++i )
            {
                FreeList::AllocBlock( &flist, claim_size, &node[i] );
            }

            for ( size_t i = 0; i < alloc_count; ++i )
            {
                FreeList::FreeBlock( &flist, node[alloc_count - i - 1] );
            }

            Assert::IsTrue( flist.total_mem == total_mem );
            Assert::IsTrue( flist.used_mem == 0 );
            Assert::IsTrue( flist.used_nodes.size == 0 );
            Assert::IsTrue( flist.free_nodes.size == 1 );

            FreeList::Destroy( &flist );

        }
        TEST_METHOD( MultipleAllocAllThenFreeAll )
        {
            CoreContext::DefaultContext();

            size_t node_cap = 5;
            size_t total_mem = 512;
            FreeList flist;

            Allocator allocator = HeapAllocator::Create();
            FreeList::Create( &flist, node_cap, total_mem, allocator );

            const size_t claim_size = 32;
            const size_t alloc_count = 16;
            FreeList::Node node[alloc_count] = {};

            for ( size_t i = 0; i < alloc_count; ++i )
            {
                FreeList::AllocBlock( &flist, claim_size, &node[i] );
            }

            for ( size_t i = 0; i < alloc_count; ++i )
            {
                FreeList::FreeBlock( &flist, node[i] );
            }

            Assert::IsTrue( flist.total_mem == total_mem );
            Assert::IsTrue( flist.used_mem == 0 );
            Assert::IsTrue( flist.used_nodes.size == 0 );
            Assert::IsTrue( flist.free_nodes.size == 1 );

            FreeList::Destroy( &flist );
        }
    };
}