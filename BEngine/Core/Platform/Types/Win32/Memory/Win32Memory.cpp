#include "Win32Memory.h"

Win32Memory::Win32Memory ( Win32Platform* platform )
{
	this->platform = platform;
	this->app = platform->app;
}
