#include "Win32Platform.h"
#include "../Window/Win32Window.h"
#include "../Input/Win32Input.h"
#include "../Memory/Win32Memory.h"
#include "../Filesystem/Win32Filesystem.h"

double Win32Platform::GetSystemTime ()
{
	LARGE_INTEGER now;
	QueryPerformanceCounter ( &now );

	return now.QuadPart * secondPerTick;
}

void Win32Platform::PlatformSleep ( unsigned long  durationInMS )
{
	Sleep(durationInMS);
}

void* Win32Platform::PlatformAllocate ( size_t size, bool aligned )
{
	return malloc(size);
}

void Win32Platform::PlatformFree ( void* const ptr, size_t size, bool aligned )
{
	free ( ptr );
}

void Win32Platform::PlatformInitMemory ( void* const src, size_t size )
{
	memset (src, 0, size );
}

void Win32Platform::PlatformCopyMemory ( void * const src, void* const dest, size_t size )
{
	memcpy ( dest,src, size );
}

void Win32Platform::PlatformSetMemory ( void* const dest, int32_t GetNewID, size_t size )
{
	memset ( dest, GetNewID, size );
}

void Win32Platform::Destroy ()
{
	window->Destroy ();
	input->Destroy ();
	memory->Destroy ();
    filesystem->Destroy ();
}

Win32Platform::Win32Platform (Application* application) : Platform(application)
{
	GetModuleHandleEx(0 , nullptr , &this->processHandle );

	this->window = new Win32Window ( this );
	this->input = new Win32Input (this);
	this->memory = new Win32Memory ( this );
    this->filesystem = new Win32Filesystem ( this );
}

void Win32Platform::Startup ()
{
	this->window->Startup ();
	this->input->Startup ();
	this->memory->Startup ();
	this->filesystem->Startup ();

	// get the frequency of the CPU
	LARGE_INTEGER frequency;

	// frequency is basically ticks per second
	QueryPerformanceFrequency ( &frequency );

	// since frequency is ticks/sec
	// if would be also useful to have sec/tick
	// that way if we wanna get the timing in seconds
	// we just need to (ticks * sec/tick) = the time passed in seconds

	this->tickPerSecond = frequency.QuadPart;
	this->secondPerTick = (double)1 / tickPerSecond;

	// get the current "tick" counter of the CPU
	// we consider this to be the "startTime" of the engine
	QueryPerformanceCounter ( &startTime );


}
