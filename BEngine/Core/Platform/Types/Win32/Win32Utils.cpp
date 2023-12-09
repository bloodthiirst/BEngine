#include "Win32Utils.h"
#include "../../../Global/Global.h"

KeydownFuncPtr* Win32Utils::keydownFuncLookup[2];
ScrollWheelFuncPtr* Win32Utils::scrollWheelLookup[2];


void Win32Utils::NoOp ( HWND windowHandle, WPARAM wParam )
{}

void Win32Utils::Keydown ( HWND windowHandle, WPARAM wParam )
{
	Application* app = (Application*) (GetWindowLongPtr ( windowHandle, GWLP_USERDATA ));
    Global::platform.input.SetKeyDown ( (KeyCode) wParam );
}

void Win32Utils::ScrollWheelPositive ( HWND windowHandle, WPARAM wParam )
{
	Application* app = (Application*) (GetWindowLongPtr ( windowHandle, GWLP_USERDATA ));
    Global::platform.input.SetScrollWheel ( 1 );
}

void Win32Utils::ScrollWheelNegative ( HWND windowHandle, WPARAM wParam )
{
	Application* app = (Application*) (GetWindowLongPtr ( windowHandle, GWLP_USERDATA ));
    Global::platform.input.SetScrollWheel ( -1 );
}

