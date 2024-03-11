#pragma once
#include "../Defines/Defines.h"
#include "../Platform/Base/Platform.h"
#include "../EventSystem/GameEventSystem.h"
#include "../Application/Application.h"
#include "../Renderer/Backend/BackendRenderer.h"

struct AllocationToolbox;
class Logger;

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

#define VK_CHECK(X , RESULT) \
        VkResult RESULT = X;\
        if(RESULT != VK_SUCCESS) {\
            Global::logger.Error( "Vulkan Error !" );\
            Global::logger.Error( #X );\
        }\

struct ArenaCheckpoint
{
    Arena* arena;
    size_t start_offset;
};

struct BAPI AllocationToolbox
{
    Arena frame_arena;
    Allocator heap_allocator;
    Allocator frame_allocator;

    template<typename T>
    T* HeapAlloc( bool init = true )
    {
        size_t size = sizeof( T );
        T* ptr = (T*) Global::platform.memory.malloc( size );

        if ( init )
        {
            Global::platform.memory.mem_init( ptr, size );
        }

        return ptr;
    }
    template<typename T>
    T* HeapAlloc( size_t count, bool init = true )
    {
        size_t size = sizeof( T ) * count;
        T* ptr = (T*) Global::platform.memory.malloc( size );

        if ( init )
        {
            Global::platform.memory.mem_init( ptr, size );
        }

        return ptr;
    }

    template<typename T>
    void HeapFree( T* ptr )
    {
        Global::platform.memory.free( (void*) ptr );
    }

    template<typename T>
    T* ArenaAlloc( bool init = true )
    {
        size_t size = sizeof( T );
        T* ptr = (T*) frame_allocator.alloc( frame_allocator, size );

        if ( init )
        {
            Global::platform.memory.mem_init( ptr, size );
        }

        return ptr;
    }

    template<typename T>
    T* ArenaAlloc( size_t count, bool init = true )
    {
        size_t size = sizeof( T ) * count;
        T* ptr = (T*) frame_allocator.alloc( frame_allocator, size );

        if ( init )
        {
            Global::platform.memory.mem_init( ptr, size );
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

    void ResetArenaOffset( ArenaCheckpoint* in_checkpoint )
    {
        in_checkpoint->arena->offset = in_checkpoint->start_offset;
    }
};

