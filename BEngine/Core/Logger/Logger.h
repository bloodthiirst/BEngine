#pragma once
#include <String/StringView.h>
#include <String/StringBuffer.h>
#include <String/StringUtils.h>
#include <Containers/HMap.h>
#include "../Defines/Defines.h"
#include "Base/ILogger.h"

struct Global;
struct ArenaCheckpoint;

class Logger
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

    BAPI void Log( StringView message );

    BAPI void Info( StringView message );

    BAPI void Warning( StringView message );

    BAPI void Error( StringView message );

    BAPI void Fatal( StringView message );

    BAPI void NewLine( size_t repeat = 1 );

#define LOG(callback)\
{\
    Allocator alloc = ArenaAllocator::Create( &CoreContext::core_arena );\
    StringBuffer str = StringUtils::Format( alloc, message, args... );\
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
