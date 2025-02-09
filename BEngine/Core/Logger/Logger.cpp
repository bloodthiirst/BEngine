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
    HMap<size_t, ILogger>::Create( &loggers, Global::alloc_toolbox.heap_allocator, 10, Hasher, Comparer );
};

void Logger::Destroy()
{
    HMap<size_t, ILogger>::Destroy( &loggers );
};

void Logger::NewLine( size_t repeat )
{
    ArenaCheckpoint c = Global::alloc_toolbox.GetArenaCheckpoint();
    DEFER([&](){ Global::alloc_toolbox.ResetArenaOffset( &c ); });

    DArray<Pair<size_t, ILogger>> tmp;
    DArray<Pair<size_t, ILogger>>::Create(loggers.count, &tmp, Global::alloc_toolbox.frame_allocator);
    HMap<size_t, ILogger>::GetAll(&loggers, &tmp);

    for (size_t i = 0; i < tmp.size; ++i)
    {
        ILogger logger = tmp.data[i].value;
        logger.new_line(&logger, repeat);
    }
};

#define LOG(callback)\
{\
    char temp_buffer[1024] = {0};\
    Arena arena = {};\
    arena.data = &temp_buffer;\
    arena.offset = 0;\
    arena.capacity = 1024;\
    \
    Allocator alloc = ArenaAllocator::Create( &arena );\
    {\
        DArray<Pair<size_t, ILogger>> tmp;\
        DArray<Pair<size_t, ILogger>>::Create( loggers.count, &tmp, alloc );\
        HMap<size_t, ILogger>::GetAll( &loggers, &tmp );\
        for ( size_t i = 0; i < tmp.size; ++i )\
        {\
            ILogger logger = tmp.data[i].value;\
            logger.callback( &logger, message );\
        }\
    }\
}\

void Logger::Log( StringView message )
{
    LOG( log )
};

void Logger::Info( StringView message )
{
    LOG( info )
};

void Logger::Warning( StringView message )
{
    LOG( warning )
};

void Logger::Error( StringView message )
{
    LOG( error )
};

void Logger::Fatal( StringView message )
{
    LOG( fatal )
};

#undef LOG