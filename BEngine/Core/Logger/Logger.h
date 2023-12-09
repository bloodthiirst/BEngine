#pragma once
#include "../Defines/Defines.h"
#include "Base/ILogger.h"
#include <typeinfo>
#include <unordered_map>
#include <cstdarg>
#include <String/StringView.h>

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
    BAPI void Log( StringView message, ... );
    BAPI void NewLine( size_t repeat = 1 );
    BAPI void Info(StringView message, ... );
    BAPI void Warning(StringView message, ... );
    BAPI void Error(StringView message, ... );
    BAPI void Fatal(StringView message, ... );
};

