#pragma once
#include <windows.h>
#include <profileapi.h>
#include <cstdint>
#include "../../Base/Time.h"
#include "../../../Global/Global.h"

struct Win32Time
{
    double seconds_per_tick;
    int64_t ticks_per_second;
    int64_t startTime;

    static void Create(Time* out_time)
    {
        Win32Time* time = (Win32Time*) Global::platform.memory.malloc(sizeof(Win32Time));

        LARGE_INTEGER frequency;
        LARGE_INTEGER start_time;

        // frequency is basically ticks per second
        QueryPerformanceFrequency(&frequency);

        // since frequency is ticks/sec
        // if would be also useful to have sec/tick
        // that way if we wanna get the timing in seconds
        // we just need to (ticks * sec/tick) = the time passed in seconds
        time->ticks_per_second = frequency.QuadPart;
        time->seconds_per_tick = 1.0 / frequency.QuadPart;

        // get the current "tick" counter of the CPU
        // we consider this to be the "startTime" of the engine
        QueryPerformanceCounter(&start_time);

        out_time->user_data = time;
        out_time->get_system_time = GetTimeWin32;
    }

    static double GetTimeWin32( Time* in_time )
    {
        LARGE_INTEGER curr_ticks;
        QueryPerformanceCounter( &curr_ticks );

        Win32Time* time = (Win32Time*) in_time->user_data;

        int64_t elapsed_ticks = curr_ticks.QuadPart - time->startTime;

        double time_elapsed = elapsed_ticks * time->seconds_per_tick;

        return time_elapsed;
    }
};