#pragma once
#include <Defines/Defines.h>
#include <Typedefs/Typedefs.h>
#include <Containers/LinkedList.h>
#include <Containers/DArray.h>
#include <Containers/Queue.h>
#include <Defer/Defer.h>
#include "../Thread/Thread.h"
#include "../AtomicLock/AtomicLock.h"

struct BAPI Job;

struct BAPI JobHandle
{
    size_t id;
};

struct BAPI Job
{
    JobHandle handle;
    LinkedList<JobHandle> dependencies;
    void* data;
    ActionParams<Job*> execute_fnc_ptr; 
};

struct BAPI JobSystem
{
    struct JobThread
    {        
        size_t index;
        Thread thread;
        Queue<Job> pending_jobs;
        AtomicLock pending_jobs_lock;
    };

    struct ThreadParams
    {
        JobThread* job_thread_ptr;
    };

    DArray<JobThread> job_threads;
    size_t thread_count;

    static void Create(size_t thread_count , JobSystem* out_js);
    static void Destroy(JobSystem* inout_js);
};