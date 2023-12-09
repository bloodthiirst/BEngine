#pragma once
#include "../Platform/Base/Platform.h"
#include "../Logger/Logger.h"
#include "../EventSystem/GameEventSystem.h"
#include "../Application/Application.h"
#include "../Renderer/Backend/BackendRenderer.h"

struct AllocationToolbox;

struct Global
{
public:
    static BAPI Application app;

    static BAPI Platform platform;

    static BAPI BackendRenderer backend_renderer;

    static BAPI Logger logger;

    static BAPI GameEventSystem event_system;

    static BAPI AllocationToolbox alloc_toolbox;
};

struct AllocationToolbox
{
    Arena frame_arena;
    Allocator heap_allocator;
    Allocator frame_allocator;

    template<typename T>
    T* HeapAlloc( bool init = true )
    {
        size_t size = sizeof( T );
        T* ptr = (T*)Global::platform.memory.malloc( size );

        if (init)
        {
            Global::platform.memory.mem_init( ptr, size );
        }

        return ptr;
    }

    template<typename T>
    T* ArenaAlloc( bool init = true )
    {
        size_t size = sizeof( T );
        T* ptr = (T*)frame_allocator.alloc( frame_allocator, size );

        if (init)
        {
            Global::platform.memory.mem_init( ptr, size );
        }

        return ptr;
    }
};

