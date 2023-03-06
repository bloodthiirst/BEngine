#pragma once
#include "../../../../Platform/Types/Win32/Platform/Win32Platform.h"
#include "../../../../Platform/Base/Memory/Memory.h"
class Application;

class Win32Memory : public Memory
{
public:
	Win32Memory ( Win32Platform* platform );
};

