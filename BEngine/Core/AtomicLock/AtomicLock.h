#pragma once
#include <windows.h>

struct AtomicLock
{
    bool is_locked;

    void Lock()
    {
        while(_InterlockedCompareExchange8((char*) &is_locked ,true , false));
        return;
    }

    void Unlock()
    {    
        _InterlockedExchange8((char*) &is_locked ,  false);
    }
};