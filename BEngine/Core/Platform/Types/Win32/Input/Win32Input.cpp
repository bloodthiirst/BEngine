#include "Win32Input.h"


Win32Input::Win32Input ( Win32Platform* platform )
{
	this->platform = platform;
	this->app = platform->app;
}