#pragma once
#include <cstdio>
#include <iostream>
#include <Containers/Queue.h>
#include "../../Thread/Thread.h"
#include "../Base/ILogger.h"

struct ConsoleLogger
{
    static const WORD DEFAULT_COLOR = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;

    struct ConsoleLoggerData
    {
        HANDLE h_stdout_console;
        HANDLE h_mutex;
        Thread logging_thread;
        Queue<StringBuffer> message_queue;
    };

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
        Thread::Create(LoggingRun , data , &data->logging_thread);
        Queue<StringBuffer>::Create(&data->message_queue , 100 , Global::alloc_toolbox.heap_allocator );

        data->h_stdout_console = GetStdHandle( STD_OUTPUT_HANDLE );
        data->h_mutex = CreateMutex(NULL,                       // default security attributes
                                        false,                  // initial count
                                        "Logging Mutex");   // unnamed semaphore
        out_logger->user_data = data;
        out_logger->log = Log;
        out_logger->info = Info;
        out_logger->error = Error;
        out_logger->new_line = NewLine;
        out_logger->warning = Warning;
        out_logger->fatal = Fatal;

        return true;
    }

    static DWORD LoggingRun(void* param)
    {   
        ConsoleLoggerData* data = (ConsoleLoggerData*) param;
        return 1;
    }

    static bool Destroy( ILogger* in_logger )
    {
        ConsoleLoggerData* data = (ConsoleLoggerData*) in_logger->user_data;
        Thread::Destroy(&data->logging_thread);
        Queue<StringBuffer>::Destroy(&data->message_queue);
        
        CloseHandle(data->h_mutex);
        CloseHandle(data->h_stdout_console);
        FreeConsole();

        return true;
    }

private:

    static void LogInternal( ILogger* in_logger, StringView message, WORD text_col )
    {
        ConsoleLoggerData* data = (ConsoleLoggerData*)in_logger->user_data;
        FlushConsoleInputBuffer( data->h_stdout_console );
        SetConsoleTextAttribute( data->h_stdout_console, text_col );

        std::cout.write(message.buffer, message.length);
        std::cout << "\n";

        SetConsoleTextAttribute( data->h_stdout_console, ConsoleLogger::DEFAULT_COLOR );
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

