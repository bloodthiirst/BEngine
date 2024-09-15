#pragma once
#include <Typedefs/Typedefs.h>
#include "Time.h"
#include "Memory.h"
#include "Filesystem.h"
#include "Window.h"
#include "Input.h"

struct BAPI Platform
{
    void* user_data;

    Time time;
    Memory memory;
    Filesystem filesystem;
	Window window;
	Input input;

	ActionParams<Platform*> startup;
    ActionParams<uint64_t> sleep;
    ActionParams<Platform*> destroy;
};

