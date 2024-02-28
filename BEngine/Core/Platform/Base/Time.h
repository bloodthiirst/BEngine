#pragma once
#include <Typedefs/Typedefs.h>

struct Time
{
    void* user_data;
    Func<double ,Time*> get_system_time;
};