#pragma once

#pragma warning( push , 1)
#include <windowsx.h>
#include <windows.h>
#pragma warning( pop )

#include "../../../Base/Platform/Platform.h"
#include "../../../Base/Window/Window.h"

class Win32Platform;

class Win32Window : public Window
{

public:
	HWND windowHandle;

	Win32Platform* platform;
public:

	Win32Window ( Win32Platform* platform );

public:
	virtual bool HandleMessages () override;
	virtual void Startup () override;
	virtual void Destroy () override;
};

