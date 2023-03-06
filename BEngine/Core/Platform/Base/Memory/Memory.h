#pragma once
#include <cstdint>

class Application;
class Platform;

enum AllocationType
{
	Generic,
	Array,
	Vector,
	Map,
	Queue,
	TextureAlloc,
	Scene,
	Rendering,

	MAX_ALLOCATION_TYPE
};

class AllocationState
{
public:
	long long totalAllocations;
	long long allocationByType[AllocationType::MAX_ALLOCATION_TYPE];
};


class Memory
{
public:

	Application* app;
    Platform* platform;
	AllocationState allocationState;

public:
	/// <summary>
	/// Handles the startup of the process based of the game setup provided by the app
	/// </summary>
	/// <returns></returns>
	virtual void Startup ();

	/// <summary>
	/// Cleanup before killing the process
	/// </summary>
	/// <returns></returns>
	virtual void Destroy ();


	virtual void* Allocate ( size_t size, AllocationType type,  bool aligned );
	virtual void Free ( void* const ptr, size_t size, AllocationType type, bool aligned );
	virtual void Init ( void* const src, size_t size );
	virtual void Copy ( void* const src, void* const dest, size_t size );
	virtual void Set ( void* const dest, int32_t value, size_t size );

};

