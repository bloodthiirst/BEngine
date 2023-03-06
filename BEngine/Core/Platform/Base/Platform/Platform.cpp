#include "Platform.h"


Platform::Platform ( Application* app)
{
	this->app = app;

	this->window = nullptr;
	this->input = nullptr;
	this->memory = nullptr;
	this->tickPerSecond = 0;
	this->secondPerTick = 0;
	this->startTime.QuadPart = 0;
}

Platform::~Platform ()
{
}
