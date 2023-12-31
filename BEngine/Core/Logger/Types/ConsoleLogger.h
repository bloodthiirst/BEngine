#pragma once
#include <cstdio>
#include <iostream>
#include "../Base/ILogger.h"

class ConsoleLogger
{
    static const WORD DEFAULT_COLOR = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;

    struct ConsoleLoggerData
    {
        HANDLE hConsole;
    };

public:
    static bool Create( ILogger* out_logger )
    {
        AllocConsole();

        FILE* fpstdin = stdin;
        FILE* fpstdout = stdout;
        FILE* fpstderr = stderr;

        freopen_s( &fpstdin, "CONIN$", "r", stdin );
        freopen_s( &fpstdout, "CONOUT$", "w", stdout );
        freopen_s( &fpstderr, "CONOUT$", "w", stderr );

        ConsoleLoggerData* data = Global::alloc_toolbox.HeapAlloc<ConsoleLoggerData>();
        data->hConsole = GetStdHandle( STD_OUTPUT_HANDLE );

        out_logger->user_data = data;
        out_logger->log = Log;
        out_logger->info = Info;
        out_logger->error = Error;
        out_logger->new_line = NewLine;
        out_logger->warning = Warning;
        out_logger->fatal = Fatal;

        return true;
    }

    static bool Destroy( ILogger* in_logger )
    {
        FreeConsole();

        return true;
    }

private:

    static void LogInternal( ILogger* in_logger, StringView message, WORD text_col )
    {
        ConsoleLoggerData* data = (ConsoleLoggerData*)in_logger->user_data;
        FlushConsoleInputBuffer( data->hConsole );
        SetConsoleTextAttribute( data->hConsole, text_col );

        std::cout.write(message, message.length);
        std::cout << "\n";

        SetConsoleTextAttribute( data->hConsole, ConsoleLogger::DEFAULT_COLOR );
    }

    static void Log( ILogger* in_logger, StringView message)
    {
        LogInternal( in_logger, message, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY );
    }

    static void Info( ILogger* in_logger, StringView message)
    {
        LogInternal( in_logger, message, FOREGROUND_BLUE | FOREGROUND_INTENSITY );
    }

    static void Warning( ILogger* in_logger, StringView message)
    {
        LogInternal( in_logger, message, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY );
    }

    static void NewLine( ILogger* in_logger, size_t repeat )
    {
        for (size_t i = 0; i < repeat; ++i) 
        {
            std::cout << "\n";
        }
    }

    static void Error( ILogger* in_logger, StringView message )
    {
        LogInternal( in_logger, message, FOREGROUND_RED | FOREGROUND_INTENSITY );
    }

    static void Fatal( ILogger* in_logger, StringView message )
    {
        LogInternal( in_logger, message, FOREGROUND_RED );
    }
};

