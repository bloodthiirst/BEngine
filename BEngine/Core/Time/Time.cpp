#include "Time.h"

Time::Time (Application* app )
{
	this->app = app;
	startupTime = { 0 };
	elapsed = { 0 };
	delta = { 0 };
}

void Time::Destroy ()
{}

Time::~Time ()
{}
