#pragma once

#include "BTest.h"
#include <Containers/FreeList.h>
#include <Allocators/Allocator.h>

namespace Tests
{
    struct FreeListTests
    {
        TEST_DECLARATION(Create)
        TEST_BODY
        ({
            CoreContext::DefaultContext();

            size_t node_cap = 5;
            size_t mem = 64;
            FreeList flist;

            Allocator allocator = HeapAllocator::Create();
            FreeList::Create( &flist, 1 , node_cap, mem, allocator );

            EVALUATE( flist.total_mem == mem );
            EVALUATE( flist.used_mem == 0 );

            FreeList::Destroy( &flist );
        })

        TEST_DECLARATION(AllocBlock)
        TEST_BODY
        ({
            CoreContext::DefaultContext();

            size_t node_cap = 5;
            size_t total_mem = 64;
            FreeList flist;

            Allocator allocator = HeapAllocator::Create();
            FreeList::Create( &flist, 1 , node_cap, total_mem, allocator );

            size_t claim_size = 32;
            FreeList::Node node = {};
            FreeList::AllocBlock( &flist, claim_size, &node );

            EVALUATE( node.size == claim_size );
            EVALUATE( node.start == 0 );

            EVALUATE( flist.total_mem == total_mem );
            EVALUATE( flist.used_mem == claim_size );
            EVALUATE( flist.used_nodes.size == 1 );
            EVALUATE( flist.free_nodes.size == 1 );

            FreeList::Destroy( &flist );
        })

        TEST_DECLARATION(MultipleAllocBlock)
        TEST_BODY
        ({
            CoreContext::DefaultContext();

            size_t node_cap = 5;
            size_t total_mem = 512;
            FreeList flist;

            Allocator allocator = HeapAllocator::Create();
            FreeList::Create( &flist, 1, node_cap, total_mem, allocator );


            const size_t claim_size = 32;
            const size_t alloc_count = 16;
            FreeList::Node node[alloc_count] = {};

            for ( size_t i = 0; i < alloc_count; ++i )
            {
                FreeList::AllocBlock( &flist, claim_size, &node[i] );
            }

            EVALUATE( flist.total_mem == total_mem );
            EVALUATE( flist.used_mem == claim_size * alloc_count );
            EVALUATE( flist.used_nodes.size == alloc_count );
            EVALUATE( flist.free_nodes.size == 0 );

            FreeList::Destroy( &flist );
        })

        TEST_DECLARATION(MultipleAllocBlockAndAFree)
        TEST_BODY
        ({
            CoreContext::DefaultContext();

            size_t node_cap = 5;
            size_t total_mem = 512;
            FreeList flist;

            Allocator allocator = HeapAllocator::Create();
            FreeList::Create( &flist, 1 , node_cap, total_mem, allocator );

            const size_t claim_size = 32;
            const size_t alloc_count = 16;
            FreeList::Node node[alloc_count] = {};

            for ( size_t i = 0; i < alloc_count; ++i )
            {
                FreeList::AllocBlock( &flist, claim_size, &node[i] );
            }

            FreeList::FreeBlock( &flist, FreeList::Node{ 32 ,32 } );
            FreeList::FreeBlock( &flist, FreeList::Node{ 0 , 32 } );

            EVALUATE( flist.total_mem == total_mem );
            EVALUATE( flist.used_mem == claim_size * (alloc_count - 2) );
            EVALUATE( flist.used_nodes.size == (alloc_count - 2) );
            EVALUATE( flist.free_nodes.size == 1 );

            FreeList::Destroy( &flist );
        })

        TEST_DECLARATION(MultipleAllocBlockAndAFreeOrdered)
        TEST_BODY
        ({
            CoreContext::DefaultContext();

            size_t node_cap = 5;
            size_t total_mem = 512;
            FreeList flist;

            Allocator allocator = HeapAllocator::Create();
            FreeList::Create( &flist, 1 , node_cap, total_mem, allocator );

            const size_t claim_size = 32;
            const size_t alloc_count = 16;
            FreeList::Node node[alloc_count] = {};

            for ( size_t i = 0; i < alloc_count; ++i )
            {
                FreeList::AllocBlock( &flist, claim_size, &node[i] );
            }

            FreeList::FreeBlock( &flist, FreeList::Node{ 32 ,32 } );
            FreeList::FreeBlock( &flist, FreeList::Node{ 64 ,32 } );

            EVALUATE( flist.total_mem == total_mem );
            EVALUATE( flist.used_mem == claim_size * (alloc_count - 2) );
            EVALUATE( flist.used_nodes.size == (alloc_count - 2) );
            EVALUATE( flist.free_nodes.size == 1 );

            FreeList::Destroy( &flist );
        })

        TEST_DECLARATION(FreeBlockThatNeedMerging)
        TEST_BODY
        ({
            CoreContext::DefaultContext();

            size_t node_cap = 5;
            size_t total_mem = 512;
            FreeList flist;

            Allocator allocator = HeapAllocator::Create();
            FreeList::Create( &flist, 1 ,  node_cap, total_mem, allocator );

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

            EVALUATE( flist.total_mem == total_mem );
            EVALUATE( flist.used_mem == claim_size * (alloc_count - 3) );
            EVALUATE( flist.used_nodes.size == (alloc_count - 3) );
            EVALUATE( flist.free_nodes.size == 2 );

            FreeList::Destroy( &flist );
        })

        TEST_DECLARATION(FreeBlockThatDoesntNeedMerging)
        TEST_BODY
        ({
            CoreContext::DefaultContext();

            size_t node_cap = 5;
            size_t total_mem = 512;
            FreeList flist;

            Allocator allocator = HeapAllocator::Create();
            FreeList::Create( &flist, 1 , node_cap, total_mem, allocator );

            const size_t claim_size = 32;
            const size_t alloc_count = 16;
            FreeList::Node node[alloc_count] = {};

            for ( size_t i = 0; i < alloc_count; ++i )
            {
                FreeList::AllocBlock( &flist, claim_size, &node[i] );
            }

            FreeList::FreeBlock( &flist, FreeList::Node{ 64 ,32 } );
            FreeList::FreeBlock( &flist, FreeList::Node{ 128 ,32 } );

            EVALUATE( flist.total_mem == total_mem );
            EVALUATE( flist.used_mem == claim_size * (alloc_count - 2) );
            EVALUATE( flist.used_nodes.size == (alloc_count - 2) );
            EVALUATE( flist.free_nodes.size == 2 );

            FreeList::Destroy( &flist );
        })

        TEST_DECLARATION(MultipleAllocBlockAndAFreeReverseOrder)
        TEST_BODY
        ({
            CoreContext::DefaultContext();

            size_t node_cap = 5;
            size_t total_mem = 512;
            FreeList flist;

            Allocator allocator = HeapAllocator::Create();
            FreeList::Create( &flist, 1 , node_cap, total_mem, allocator );

            const size_t claim_size = 32;
            const size_t alloc_count = 16;
            FreeList::Node node[alloc_count] = {};

            for ( size_t i = 0; i < alloc_count; ++i )
            {
                FreeList::AllocBlock( &flist, claim_size, &node[i] );
            }

            FreeList::FreeBlock( &flist, FreeList::Node{ 64 ,32 } );
            FreeList::FreeBlock( &flist, FreeList::Node{ 32 ,32 } );

            EVALUATE( flist.total_mem == total_mem );
            EVALUATE( flist.used_mem == claim_size * (alloc_count - 2) );
            EVALUATE( flist.used_nodes.size == (alloc_count - 2) );
            EVALUATE( flist.free_nodes.size == 1 );

            FreeList::Destroy( &flist );
        })


        TEST_DECLARATION(MultipleAllocAllThenFreeAllInReverse)
        TEST_BODY
        ({
            CoreContext::DefaultContext();

            size_t node_cap = 5;
            size_t total_mem = 512;
            FreeList flist;

            Allocator allocator = HeapAllocator::Create();
            FreeList::Create( &flist, 1 , node_cap, total_mem, allocator );

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

            EVALUATE( flist.total_mem == total_mem );
            EVALUATE( flist.used_mem == 0 );
            EVALUATE( flist.used_nodes.size == 0 );
            EVALUATE( flist.free_nodes.size == 1 );

            FreeList::Destroy( &flist );

        })

        TEST_DECLARATION(MultipleAllocAllThenFreeAll)
        TEST_BODY
        ({
            CoreContext::DefaultContext();

            size_t node_cap = 5;
            size_t total_mem = 512;
            FreeList flist;

            Allocator allocator = HeapAllocator::Create();
            FreeList::Create( &flist, 1 , node_cap, total_mem, allocator );

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

            EVALUATE( flist.total_mem == total_mem );
            EVALUATE( flist.used_mem == 0 );
            EVALUATE( flist.used_nodes.size == 0 );
            EVALUATE( flist.free_nodes.size == 1 );

            FreeList::Destroy( &flist );
        })

        static inline DArray<TestCallback> GetAll() 
        {
            Allocator alloc = HeapAllocator::Create();
            DArray<TestCallback> arr = {};
            DArray<TestCallback>::Create(10 , &arr , alloc);

            DArray<TestCallback>::Add(&arr , FreeListTests::Create);
            DArray<TestCallback>::Add(&arr , FreeListTests::AllocBlock);
            DArray<TestCallback>::Add(&arr , FreeListTests::FreeBlockThatDoesntNeedMerging);
            DArray<TestCallback>::Add(&arr , FreeListTests::FreeBlockThatNeedMerging);
            DArray<TestCallback>::Add(&arr , FreeListTests::MultipleAllocAllThenFreeAll);
            DArray<TestCallback>::Add(&arr , FreeListTests::MultipleAllocAllThenFreeAllInReverse);
            DArray<TestCallback>::Add(&arr , FreeListTests::MultipleAllocBlock);
            DArray<TestCallback>::Add(&arr , FreeListTests::MultipleAllocBlockAndAFree);
            DArray<TestCallback>::Add(&arr , FreeListTests::MultipleAllocBlockAndAFreeOrdered);
            DArray<TestCallback>::Add(&arr , FreeListTests::MultipleAllocBlockAndAFreeReverseOrder);

            return arr;
        }; 
    };
}