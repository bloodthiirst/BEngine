#pragma once
#include <Windows.h>
#include "../../../Application/Application.h"

typedef void(KeydownFuncPtr) ( HWND, WPARAM );
typedef void(ScrollWheelFuncPtr) ( HWND, WPARAM );

class Win32Utils
{

public:
	static KeydownFuncPtr* keydownFuncLookup[2];
	static ScrollWheelFuncPtr* scrollWheelLookup[2];

public:

	static void NoOp ( HWND windowHandle, WPARAM wParam );

	static void Keydown ( HWND windowHandle, WPARAM wParam );
	static void ScrollWheelPositive ( HWND windowHandle, WPARAM wParam );
	static void ScrollWheelNegative ( HWND windowHandle, WPARAM wParam );
};