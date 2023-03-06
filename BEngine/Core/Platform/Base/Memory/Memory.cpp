#include "Memory.h"
#include "../../../Application/Application.h"
#include "../Platform/Platform.h"

void Memory::Startup ()
{
	allocationState = AllocationState ();
	allocationState.totalAllocations = 0;

	for(int i = 0 ; i < AllocationType::MAX_ALLOCATION_TYPE;++i )
	{
		allocationState.allocationByType[i] = 0;
	}
}

void Memory::Destroy ()
{}

void* Memory::Allocate ( size_t size, AllocationType type, bool aligned )
{
	allocationState.totalAllocations += size;
	allocationState.allocationByType[type] += size;

	void* mem = platform->PlatformAllocate ( size, aligned );
	platform->PlatformInitMemory ( mem, size );

	return mem;
}

void Memory::Free ( void* const ptr, size_t size, AllocationType type, bool aligned )
{
	allocationState.totalAllocations -= size;
	allocationState.allocationByType[type] -= size;

	return platform->PlatformFree (ptr, size, aligned );
}

void Memory::Init ( void* const src, size_t size )
{
	platform->PlatformInitMemory ( src, size );
}

void Memory::Copy ( void* const src, void* const dest, size_t size )
{
	platform->PlatformCopyMemory ( src, dest, size );
}

void Memory::Set ( void* const dest, int32_t value, size_t size )
{
	platform->PlatformSetMemory ( dest, value, size );
}
