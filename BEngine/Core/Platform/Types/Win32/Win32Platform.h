#pragma once
#include "../../Base/Platform.h"
#include "../../Base/Input.h"
#include "Win32Time.h"
#include "Win32Memory.h"
#include "Win32Window.h"
#include "Win32Filesystem.h"

struct Win32Platform
{
    static void GetStartupArgs( char** argv, int argc, ApplicationStartup* out_startup )
    {
        Allocator heap_alloc = HeapAllocator::Create();

        DArray<StringBuffer> args;
        DArray<StringBuffer>::Create( argc, &args, heap_alloc );

        for (size_t i = 0; i < argc; ++i)
        {
            char* curr_arg = argv[i];
            StringBuffer arg = StringBuffer::Create( curr_arg, heap_alloc );

            DArray<StringBuffer>::Add( &args, arg);
        }

        StringView path = args.data[0].view;

        // find the client dll
        *out_startup = {};
        out_startup->executable_folder = StringUtils::GetFolder( path, heap_alloc ).view;
        out_startup->executable_name = StringUtils::GetFilename( path, heap_alloc ).view;
        out_startup->window_rect.x = 300;
        out_startup->window_rect.y = 300;
        out_startup->window_rect.width = 500;
        out_startup->window_rect.height = 500;
    }

    static void Create(Platform* platform_out)
    {
        Input::Create(&platform_out->input);
        Win32Memory::Create(&platform_out->memory);
        Win32Filesystem::Create(&platform_out->filesystem);
        Win32Time::Create(&platform_out->time);
        Win32Window::Create(&platform_out->window);

        platform_out->sleep = Win32Sleep;
    }

    static void Win32Sleep(uint64_t milliseconds)
    {
        Sleep((DWORD) milliseconds);
    }
};