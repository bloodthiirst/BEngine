#pragma once
#include <windows.h>
#include <profileapi.h>
#include <cstdint>
#include "../../Base/Time.h"

struct Win32Time
{
    double seconds_per_tick;
    int64_t ticks_per_second;
    int64_t startTime;

    static void Create(Time* out_time)
    {
        Win32Time* time = new Win32Time();

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
    }
};