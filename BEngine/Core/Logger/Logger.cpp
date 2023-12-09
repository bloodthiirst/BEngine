#include "Logger.h"

void Logger::Initialize()
{
    loggers = std::unordered_map<size_t, ILogger>();
}

void Logger::Destroy()
{
    loggers.clear();
};

BAPI void Logger::Log( const char* message, ... )
{
    va_list args;
    va_start( args, message );

    for (auto& [key, logger] : loggers)
    {
        logger.log( &logger, message );
    }

    va_end( args );
}

void Logger::NewLine( size_t repeat )
{
    for (auto& [key, logger] : loggers)
    {
        logger.new_line( &logger, repeat );
    }
}

void Logger::Info( const char* message, ... )
{
    va_list args;
    va_start( args, message );

    for (auto& [key, logger] : loggers)
    {
        logger.info( &logger, message );
    }

    va_end( args );
}

void Logger::Warning( const char* message, ... )
{
    va_list args;
    va_start( args, message );

    for (auto& [key, logger] : loggers)
    {
        logger.warning( &logger, message );
    }

    va_end( args );
}

void Logger::Error( const char* message, ... )
{
    va_list args;
    va_start( args, message );

    for (auto& [key, logger] : loggers)
    {
        logger.fatal( &logger, message );
    }

    va_end( args );
}

void Logger::Fatal( const char* message, ... )
{
    va_list args;
    va_start( args, message );

    for (auto& [key, logger] : loggers)
    {
        logger.fatal( &logger, message );
    }

    va_end( args );
}
