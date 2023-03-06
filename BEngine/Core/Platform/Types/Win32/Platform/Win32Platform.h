#pragma once
#include "../../../Base/Platform/Platform.h"

class Win32Platform : public Platform
{
public:
	HINSTANCE processHandle;
public:
	Win32Platform ( Application* application );

public:
	virtual void Startup () override;

	/// <summary>
	/// Time in seconds
	/// </summary>
	/// <returns></returns>
	virtual double GetSystemTime () override;

	/// <summary>
	/// Set the platform thread to sleep for durationInMS
	/// </summary>
	/// <param name="durationInMS"></param>
	virtual void PlatformSleep ( unsigned long  durationInMS ) override;

	/// <summary>
	/// Allocate a chunck of memory from the OS
	/// </summary>
	/// <param name="size"></param>
	/// <param name="aligned"></param>
	/// <returns></returns>
	virtual void* PlatformAllocate ( size_t size, bool aligned ) override;
	virtual void PlatformFree ( void* const ptr, size_t size ,bool aligned ) override;
	virtual void PlatformInitMemory ( void* const src, size_t size ) override;
	virtual void PlatformCopyMemory ( void* const src, void* const dest, size_t size ) override;
	virtual void PlatformSetMemory ( void* const dest, int32_t GetNewID, size_t size ) override;

	virtual void Destroy () override;


};

