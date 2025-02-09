#pragma once
#include <Allocators/Allocator.h>
#include "../Defines/Defines.h"
#include "../Platform/Base/Platform.h"
#include "../EventSystem/GameEventSystem.h"
#include "../Application/Application.h"
#include "../Renderer/Backend/BackendRenderer.h"
#include "../AssetManager/GlobalAssetManager.h"
#include "../FileWatcher/FileWatcher.h"
#include "../Thread/Thread.h"
#include "../AtomicLock/AtomicLock.h"
#include "../JobSystem/JobSystem.h"

struct BAPI GlobalAssetManager;
struct BAPI AllocationToolbox;
struct BAPI Logger;

struct FileWatchingContext
{
    FileWatcher file_watcher;
    Thread watcher_thread;
    AtomicLock queue_access_lock;
};

struct BAPI Global
{
    static Application app;

    static Platform platform;

    static BackendRenderer backend_renderer;

    static Logger logger;

    static GameEventSystem event_system;

    static AllocationToolbox alloc_toolbox;

    static GlobalAssetManager asset_manager;

    static FileWatchingContext filewatch_ctx;

    static JobSystem job_system;
};

struct BAPI AllocationToolbox
{
    Arena frame_arena;
    Allocator heap_allocator;
    Allocator frame_allocator;

    template <typename T>
    T *HeapAlloc(bool init = true)
    {
        size_t size = sizeof(T);
        T *ptr = (T *) ALLOC(heap_allocator, size);

        if (init)
        {
            Global::platform.memory.mem_init(ptr, size);
        }

        return ptr;
    }
    template <typename T>
    T *HeapAlloc(size_t count, bool init = true)
    {
        size_t size = sizeof(T) * count;
        T *ptr = (T *) ALLOC(heap_allocator, size);

        if (init)
        {
            Global::platform.memory.mem_init(ptr, size);
        }

        return ptr;
    }

    template <typename T>
    void HeapFree(T *ptr)
    {
        FREE(heap_allocator , (void *)ptr);
    }

    template <typename T>
    T *ArenaAlloc(bool init = true)
    {
        size_t size = sizeof(T);
        T *ptr = (T *)ALLOC(frame_allocator, size);

        if (init)
        {
            Global::platform.memory.mem_init(ptr, size);
        }

        return ptr;
    }

    template <typename T>
    T *ArenaAlloc(size_t count, bool init = true)
    {
        size_t size = sizeof(T) * count;
        T *ptr = (T *)ALLOC(frame_allocator, size);

        if (init)
        {
            Global::platform.memory.mem_init(ptr, size);
        }

        return ptr;
    }

    ArenaCheckpoint GetArenaCheckpoint()
    {
        ArenaCheckpoint res = {};
        res.arena = &Global::alloc_toolbox.frame_arena;
        res.start_offset = Global::alloc_toolbox.frame_arena.offset;
        return res;
    }

    void ResetArenaOffset(ArenaCheckpoint *in_checkpoint)
    {
        in_checkpoint->arena->offset = in_checkpoint->start_offset;
    }
};
