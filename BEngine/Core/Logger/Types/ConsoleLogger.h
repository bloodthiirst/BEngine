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
        Queue<StringView> message_queue;
        Arena buffer_memory;
    };

    static bool Create(ILogger *out_logger)
    {
        AllocConsole();

        FILE *fpstdin = stdin;
        FILE *fpstdout = stdout;
        FILE *fpstderr = stderr;

        freopen_s(&fpstdin, "CONIN$", "r", stdin);
        freopen_s(&fpstdout, "CONOUT$", "w", stdout);
        freopen_s(&fpstderr, "CONOUT$", "w", stderr);

        ConsoleLoggerData *data = Global::alloc_toolbox.HeapAlloc<ConsoleLoggerData>();
        Thread::Create(LoggingRun, data, &data->logging_thread);
        Queue<StringView>::Create(&data->message_queue, 100, Global::alloc_toolbox.heap_allocator);

        {
            const size_t buffer_size = 50 * 1'024'000;
            char *buffer_mem = (char *)ALLOC(Global::alloc_toolbox.heap_allocator, buffer_size);
            Arena buffer_arena = {};
            buffer_arena.capacity = buffer_size;
            buffer_arena.data = buffer_mem;
            buffer_arena.offset = 0;

            data->buffer_memory = buffer_arena;
        }

        data->h_stdout_console = GetStdHandle(STD_OUTPUT_HANDLE);
        data->h_mutex = CreateMutex(NULL, false, "Logging Mutex");

        out_logger->user_data = data;
        out_logger->log = Log;
        out_logger->info = Info;
        out_logger->error = Error;
        out_logger->new_line = NewLine;
        out_logger->warning = Warning;
        out_logger->fatal = Fatal;

        Thread::Run(&data->logging_thread);

        return true;
    }

    static DWORD LoggingRun(void *param)
    {
        ConsoleLoggerData *data = (ConsoleLoggerData *)param;

        while (true)
        {
            DWORD wait_mutex = WaitForSingleObject(data->h_mutex, INFINITE);
            {
                assert(wait_mutex == WAIT_OBJECT_0);

                StringView message = {};
                while (Queue<StringView>::TryDequeue(&data->message_queue, &message))
                {
                    WORD text_col = DEFAULT_COLOR;
                    FlushConsoleInputBuffer(data->h_stdout_console);
                    SetConsoleTextAttribute(data->h_stdout_console, text_col);

                    std::cout.write(message.buffer, message.length);
                    std::cout << "\n";

                    SetConsoleTextAttribute(data->h_stdout_console, ConsoleLogger::DEFAULT_COLOR);
                }

                Arena::Reset(&data->buffer_memory);
            }
            ReleaseMutex(data->h_mutex);
        }
        return 1;
    }

    static bool Destroy(ILogger *in_logger)
    {
        ConsoleLoggerData *data = (ConsoleLoggerData *)in_logger->user_data;
        Thread::Suspend(&data->logging_thread);
        Thread::Destroy(&data->logging_thread);
        Queue<StringView>::Destroy(&data->message_queue);

        Global::alloc_toolbox.HeapFree(data);

        CloseHandle(data->h_mutex);
        CloseHandle(data->h_stdout_console);
        FreeConsole();

        return true;
    }

private:
    static void LogInternal(ILogger *in_logger, StringView message, WORD text_col)
    {
        ConsoleLoggerData *data = (ConsoleLoggerData *)in_logger->user_data;

        Allocator alloc = ArenaAllocator::Create(&data->buffer_memory);

        DWORD wait_mutex = WaitForSingleObject(data->h_mutex, INFINITE);
        {
            assert(wait_mutex == WAIT_OBJECT_0);
            StringBuffer str = StringBuffer::Create(message, alloc);
            Queue<StringView>::Enqueue(&data->message_queue, str.view);
        }
        ReleaseMutex(data->h_mutex);
    }

    static void Log(ILogger *in_logger, StringView message)
    {
        LogInternal(in_logger, message, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    }

    static void Info(ILogger *in_logger, StringView message)
    {
        LogInternal(in_logger, message, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    }

    static void Warning(ILogger *in_logger, StringView message)
    {
        LogInternal(in_logger, message, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    }

    static void Error(ILogger *in_logger, StringView message)
    {
        LogInternal(in_logger, message, FOREGROUND_RED | FOREGROUND_INTENSITY);
    }

    static void Fatal(ILogger *in_logger, StringView message)
    {
        LogInternal(in_logger, message, FOREGROUND_RED);
    }

    static void NewLine(ILogger *in_logger, size_t repeat)
    {
        ConsoleLoggerData *data = (ConsoleLoggerData *)in_logger->user_data;

        Allocator alloc = STACK_ALLOC(sizeof(StringView) + (repeat * sizeof(char)));
        char *str = (char *)ALLOC(alloc, repeat * sizeof(char));
        StringView view = {};
        view.buffer = str;
        view.length = repeat;

        for (size_t i = 0; i < repeat; ++i)
        {
            str[i] = '\n';
        }

        LogInternal(in_logger, view, DEFAULT_COLOR);
    }
};
