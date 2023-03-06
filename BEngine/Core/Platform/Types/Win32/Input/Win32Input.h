#pragma once
#include "../../../Base/Input/Input.h"
#include "../Platform/Win32Platform.h"

class Win32Input : public Input
{
public:

	Win32Platform* platform;
public:
	Win32Input ( Win32Platform* platform );
};

