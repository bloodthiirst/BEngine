#include <Typedefs/Typedefs.h>
#include <Containers/LinkedList.h>
#include <Containers/DArray.h>
#include <Containers/Queue.h>
#include <Defer/Defer.h>
#include "../Thread/Thread.h"
#include "../AtomicLock/AtomicLock.h"
#include "../Global/Global.h"
#include "JobSystem.h"

DWORD ThreadRun(void* data)
{
    JobSystem::ThreadParams th = *((JobSystem::ThreadParams*) data);

    char buffer[1024] = {0};
    
    Arena arena = {};
    arena.data = &buffer;
    arena.capacity = 1024;
    arena.offset = 0;

    Allocator alloc = ArenaAllocator::Create(&arena);

    while(true)
    {
        DEFER([&](){ Arena::Reset(&arena); });

        StringBuffer str = StringUtils::Format(alloc , alloc, "Thread index [{}]" , th.job_thread_ptr->index);
        Global::logger.Log(str.view);
        Global::platform.sleep(1000);
    }

    return 0;
}

void JobSystem::Create(size_t thread_count , JobSystem* out_js)
{
    *out_js = {};
    out_js->thread_count = thread_count;
    DArray<JobThread>::Create(thread_count , &out_js->job_threads , Global::alloc_toolbox.heap_allocator);

    for(size_t i = 0; i < out_js->thread_count; ++i)
    {
        ThreadParams* params = Global::alloc_toolbox.HeapAlloc<ThreadParams>();
        
        JobThread th = {};
        th.index = i;
        Queue<Job>::Create(&th.pending_jobs , 20 , Global::alloc_toolbox.heap_allocator);
        Thread::Create(ThreadRun , params , &th.thread);
        
        DArray<JobThread>::Add(&out_js->job_threads , th);
        size_t idx = out_js->job_threads.size - 1;

        params->job_thread_ptr = &out_js->job_threads.data[idx];

        th.thread.Run(&th.thread);
    }
}

void JobSystem::Destroy(JobSystem* inout_js)
{
    for(size_t i = 0; i < inout_js->job_threads.size; ++i)
    {
        JobThread* curr_th = &inout_js->job_threads.data[i];
        
        Thread::Suspend(&curr_th->thread);
        Thread::Destroy(&curr_th->thread);
        Queue<Job>::Destroy(&curr_th->pending_jobs);
    }

    DArray<JobThread>::Destroy(&inout_js->job_threads);
    *inout_js = {};
}