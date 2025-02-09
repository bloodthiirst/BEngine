#pragma once
#include <String/StringView.h>
#include <String/StringBuffer.h>
#include <String/StringUtils.h>
#include <Containers/HMap.h>
#include "../Defines/Defines.h"
#include "Base/ILogger.h"

struct Global;
struct ArenaCheckpoint;

class BAPI Logger
{
private:
    HMap<size_t, ILogger> loggers;

public:
    void Initialize();
    void Destroy();

    size_t Add( ILogger in_logger )
    {
        const size_t id = loggers.count;

        size_t out_index;

        HMap<size_t, ILogger>::TryAdd( &loggers, id, in_logger, &out_index );

        return id;
    };

    bool Remove( size_t id )
    {
        ILogger removed = {};

        return HMap<size_t, ILogger>::TryRemove( &loggers, id, &removed );
    };

    void Log( StringView message );

    void Info( StringView message );

    void Warning( StringView message );

    void Error( StringView message );

    void Fatal( StringView message );

    void NewLine( size_t repeat = 1 );

#define LOG(callback)             \
{                                 \
    char temp_buffer[1024] = {0}; \
    Arena arena = {};             \
    arena.data = &temp_buffer;    \
    arena.capacity = 1024;        \
    arena.offset = 0;             \
    Allocator temp_alloc = ArenaAllocator::Create(&arena);\
    \
    Allocator alloc = ArenaAllocator::Create( &CoreContext::core_arena );\
    StringBuffer str = StringUtils::Format( alloc, temp_alloc, message, args... );\
    ArenaCheckpoint c = Global::alloc_toolbox.GetArenaCheckpoint();\
    {\
        DArray<Pair<size_t, ILogger>> tmp;\
        DArray<Pair<size_t, ILogger>>::Create( loggers.count, &tmp, Global::alloc_toolbox.frame_allocator );\
        HMap<size_t, ILogger>::GetAll( &loggers, &tmp );\
        for ( size_t i = 0; i < tmp.size; ++i )\
        {\
            ILogger logger = tmp.data[i].value;\
            logger.callback( &logger, str.view );\
        }\
    }\
    \
    Global::alloc_toolbox.ResetArenaOffset( &c );\
}\

    template<typename ...Args>
    void  Log( StringView message, Args... args )
    {
        LOG( log );
    };

    template<typename ...Args>
    void  Info( StringView message, Args... args )
    {
        LOG( info );
    };

    template<typename ...Args>
    void  Warning( StringView message, Args... args )
    {
        LOG( warning );
    };

    template<typename ...Args>
    void  Error( StringView message, Args... args )
    {
        LOG( error );
    };

    template<typename ...Args>
    void Fatal( StringView message, Args... args )
    {
        LOG(fatal)
    };

#undef LOG
};
