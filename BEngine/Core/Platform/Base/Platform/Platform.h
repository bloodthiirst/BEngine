#pragma once
#include <vulkan/vulkan.h>
#include "../Window/Window.h"
#include "../Input/Input.h"
#include <GameApp.h>
#include "../Memory/Memory.h"
#include "../Filesystem/Filesystem.h"

class Application;

class Platform
{
public:
	// todo : move this to a "clock" class
	double secondPerTick;
	long long tickPerSecond;
	LARGE_INTEGER startTime;

public:
	Application* app;
	Window* window;
	Input* input;
	Memory* memory;
    Filesystem* filesystem;

public:

	Platform ( Application* app);

public:
	virtual void Startup () = 0;
	virtual double GetSystemTime () = 0;
	virtual void PlatformSleep ( unsigned long durationInMS ) = 0;
	
	virtual void* PlatformAllocate ( size_t size  , bool aligned) = 0;
	virtual void PlatformFree ( void* const ptr, size_t size, bool aligned ) = 0;
	virtual void PlatformInitMemory ( void* const src ,size_t size ) = 0;
	virtual void PlatformCopyMemory ( void* const src , void* const dest ,size_t size ) = 0;
	virtual void PlatformSetMemory ( void* const dest, int32_t value , size_t size ) = 0;

	virtual void Destroy () = 0;

	~Platform ();
};

