#pragma once
#include <windows.h>
#include <stdint.h>
#include <Typedefs/Typedefs.h>
#include <assert.h>

struct Thread
{
    typedef Func<DWORD , void*> ThreadCallback;
    
    DWORD thread_id;
    HANDLE thread_handle; 
    void* callback_param;
    Func<DWORD , void*> thread_callback;

    static void Create(ThreadCallback callback , void* callback_param, Thread* out_thread)
    {
        DWORD creation_flag = CREATE_SUSPENDED;
        out_thread->callback_param = callback_param;
        out_thread->thread_callback = callback;
        out_thread->thread_handle = CreateThread(nullptr , 0 , callback ,  callback_param , creation_flag,  &out_thread->thread_id);
    }

    static void Run(Thread* in_thread)
    {
        assert(ResumeThread(in_thread->thread_handle) != -1);
    }

    static void Suspend(Thread* in_thread)
    {
        assert(SuspendThread(in_thread->thread_handle) != -1);
    }

    static void Destroy(Thread* inout_thread)
    {
        CloseHandle(inout_thread->thread_handle);
        *inout_thread = {};    
    }
};