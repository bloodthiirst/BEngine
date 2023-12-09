#pragma once
#include <stdint.h>

struct Application;

class Time
{
public:
	Application* app;
	int64_t startupTime;
	int64_t elapsed;
	int64_t  delta;

public:
	Time ( Application* app );

	void Destroy ();
	~Time ();
};

