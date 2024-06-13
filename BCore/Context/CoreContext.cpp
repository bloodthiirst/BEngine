#include "CoreContext.h"
#include "../Allocators/Allocator.h"

Func<void*, size_t> CoreContext::malloc;

Func<void*, void*, size_t> CoreContext::realloc;

ActionParams<void*> CoreContext::free;

ActionParams<void*, size_t> CoreContext::mem_init;

Func<bool, void*, void*, size_t> CoreContext::mem_compare;

ActionParams<void*, int32_t, size_t> CoreContext::mem_set;

ActionParams<void*, void*, size_t> CoreContext::mem_copy;

ActionParams<void*, void*, size_t> CoreContext::mem_move;

Arena CoreContext::core_arena;

void CoreContext::DefaultContext()
{
    CoreContext::malloc = ::malloc;
    CoreContext::realloc = ::realloc;
    CoreContext::free = ::free;
    CoreContext::mem_compare = Win32MemCompare;
    CoreContext::mem_copy = Win32MemCopy;
    CoreContext::mem_move = Win32MemMove;
    CoreContext::mem_init = Win32MemInit;
    CoreContext::mem_set = Win32MemSet;
    CoreContext::core_arena = Arena::Create( 30 * 1'024'000 );
}