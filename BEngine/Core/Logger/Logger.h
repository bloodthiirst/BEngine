#pragma once
#include "../Defines/Defines.h"
#include "Base/ILogger.h"
#include <typeinfo>
#include <unordered_map>
#include <cstdarg>
#include <String/StringView.h>
#include <String/StringUtils.h>

class Logger
{
private:
    std::unordered_map<size_t, ILogger> loggers;

public:
    void Initialize();
    void Destroy();

public:
    size_t Add( ILogger in_logger )
    {
        const size_t id = loggers.size();
        auto pair = std::make_pair( id, in_logger );
        loggers.insert( pair );

        return id;
    }

    bool Remove( size_t id )
    {
        return loggers.erase( id );
    }

public:

    BAPI void Log( StringView message)
    {
        for ( size_t i = 0; i < loggers.size(); ++i )
        {
            ILogger logger = loggers.at( i );
            logger.log( &logger, message );
        }
    }

    BAPI void Info( StringView message )
    {
        for ( size_t i = 0; i < loggers.size(); ++i )
        {
            ILogger logger = loggers.at( i );
            logger.info( &logger, message );
        }
    }

    BAPI void Warning( StringView message )
    {
        for ( size_t i = 0; i < loggers.size(); ++i )
        {
            ILogger logger = loggers.at( i );
            logger.warning( &logger, message );
        }
    }

    BAPI void Error( StringView message )
    {
        for ( size_t i = 0; i < loggers.size(); ++i )
        {
            ILogger logger = loggers.at( i );
            logger.error( &logger, message );
        }
    }

    BAPI void Fatal( StringView message)
    {
        for ( size_t i = 0; i < loggers.size(); ++i )
        {
            ILogger logger = loggers.at( i );
            logger.fatal( &logger, message );
        }
    }

    template<typename ...Args>
    BAPI void Log( StringView message, Args... args )
    {
        Allocator alloc = ArenaAllocator::Create( &CoreContext::core_arena );

        StringBuffer str = StringUtils::Format( alloc, message, args... );

        for ( size_t i = 0; i < loggers.size(); ++i )
        {
            ILogger logger = loggers.at( i );
            logger.log( &logger, str.view );
        }
    }

    BAPI void NewLine( size_t repeat = 1 )
    {
        for ( auto& [key, logger] : loggers )
        {
            logger.new_line( &logger, repeat );
        }
    }

    template<typename ...Args>
    BAPI void Info(StringView message, Args... args )
    {
        Allocator alloc = ArenaAllocator::Create( &CoreContext::core_arena );

        StringBuffer str = StringUtils::Format( alloc, message, args... );

        for ( size_t i = 0; i < loggers.size(); ++i )
        {
            ILogger logger = loggers.at( i );
            logger.info( &logger, str.view );
        }
    }

    template<typename ...Args>
    BAPI void Warning(StringView message, Args... args )
    {
        Allocator alloc = ArenaAllocator::Create( &CoreContext::core_arena );

        StringBuffer str = StringUtils::Format( alloc, message, args... );

        for ( size_t i = 0; i < loggers.size(); ++i )
        {
            ILogger logger = loggers.at( i );
            logger.warning( &logger, str.view );
        }
    }


    template<typename ...Args>
    BAPI void Error(StringView message, Args... args )
    {
        Allocator alloc = ArenaAllocator::Create( &CoreContext::core_arena );

        StringBuffer str = StringUtils::Format( alloc, message, args... );

        for ( size_t i = 0; i < loggers.size(); ++i )
        {
            ILogger logger = loggers.at( i );
            logger.error( &logger, str.view );
        }
    }

    template<typename ...Args>
    BAPI void Fatal(StringView message, Args... args )
    {
        Allocator alloc = ArenaAllocator::Create( &CoreContext::core_arena );

        StringBuffer str = StringUtils::Format( alloc, message, args... );

        for ( size_t i = 0; i < loggers.size(); ++i )
        {
            ILogger logger = loggers.at( i );
            logger.fatal( &logger, str.view );
        }
    }
};

