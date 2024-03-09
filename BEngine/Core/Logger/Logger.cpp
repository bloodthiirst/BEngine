#include "Logger.h"
#include "../Global/Global.h"
#include <String/StringUtils.h>

bool Comparer( size_t a, size_t b )
{
    return a == b;
};

size_t Hasher( size_t a )
{
    return a;
};

void Logger::Initialize()
{
    HMap<size_t, ILogger>::Create( &loggers, Global::alloc_toolbox.heap_allocator, 0, 10, Hasher, Comparer );
};

void Logger::Destroy()
{
    HMap<size_t, ILogger>::Destroy( &loggers );
};


BAPI void Logger::NewLine( size_t repeat )
{
    ArenaCheckpoint c = Global::alloc_toolbox.GetArenaCheckpoint();
    {
        DArray<Pair<size_t, ILogger>> tmp;
        DArray<Pair<size_t, ILogger>>::Create( loggers.count, &tmp, Global::alloc_toolbox.frame_allocator );
        HMap<size_t, ILogger>::GetAll( &loggers, &tmp );

        for ( size_t i = 0; i < tmp.size; ++i )
        {
            ILogger logger = tmp.data[i].value;
            logger.new_line( &logger, repeat );
        }
    }
    Global::alloc_toolbox.ResetArenaOffset( &c );
};

#define LOG(callback)\
{\
    Allocator alloc = ArenaAllocator::Create( &CoreContext::core_arena );\
    ArenaCheckpoint c = Global::alloc_toolbox.GetArenaCheckpoint();\
    {\
        DArray<Pair<size_t, ILogger>> tmp;\
        DArray<Pair<size_t, ILogger>>::Create( loggers.count, &tmp, Global::alloc_toolbox.frame_allocator );\
        HMap<size_t, ILogger>::GetAll( &loggers, &tmp );\
        for ( size_t i = 0; i < tmp.size; ++i )\
        {\
            ILogger logger = tmp.data[i].value;\
            logger.callback( &logger, message );\
        }\
    }\
    \
    Global::alloc_toolbox.ResetArenaOffset( &c );\
}\

BAPI void Logger::Log( StringView message )
{
    LOG( log )
};

BAPI void Logger::Info( StringView message )
{
    LOG( info )
};

BAPI void Logger::Warning( StringView message )
{
    LOG( warning )
};

BAPI void Logger::Error( StringView message )
{
    LOG( error )
};

BAPI void Logger::Fatal( StringView message )
{
    LOG( fatal )
};

#undef LOG