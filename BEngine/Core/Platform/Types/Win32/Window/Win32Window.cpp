#pragma once
#include "Win32Utils.h"
#include "../../../../Application/Application.h"
#include "../Platform/Win32Platform.h"
#include "Win32Window.h"

Win32Window::Win32Window ( Win32Platform* platform )
{
	this->windowHandle = nullptr;
	this->platform = platform;
	this->app = platform->app;
}

bool Win32Window::HandleMessages ()
{
	MSG msg = { 0 };
	bool hasQuit = false;

	while ( PeekMessage ( &msg, NULL, 0, 0, PM_REMOVE ) )
	{
		TranslateMessage ( &msg );

		DispatchMessage ( &msg );

		hasQuit |= msg.message == WM_QUIT;

		Sleep ( 1 );
	}

	return !hasQuit;
}

inline void HandleEngineClose ( EngineCloseEvent evt )
{
	HWND hwnd = ((Win32Window*) evt.app->platform->window)->windowHandle;
	DestroyWindow ( hwnd );
}


/// <summary>
/// Handle the OS messages
/// </summary>
/// <param name="hwnd"></param>
/// <param name="uMsg"></param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns>returning 1 means the message has been handled</returns>
LRESULT CALLBACK HandleMessage ( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch ( uMsg )
	{
		case WM_DESTROY:
		{
			PostQuitMessage ( 0 );
			return 0;
		}
		case WM_CLOSE:
		{
			if ( MessageBox ( hwnd, TEXT ( "Are you sure you want to exit?" ), TEXT ( "Exit" ), MB_YESNO | MB_ICONQUESTION ) == IDYES )
			{
				Application* app = (Application*) (GetWindowLongPtr ( hwnd, GWLP_USERDATA ));
				EngineCloseEvent evt = EngineCloseEvent ();
				evt.app = app;

				app->gameEventSystem.Trigger<EngineCloseEvent> ( evt );
			}

			return 0;
		}

		case WM_DROPFILES:
		{
			return 0;
		}

		case WM_ERASEBKGND:
		{
			return 0;
		}

		case WM_SIZE:
		{
			RECT size;

			// get the new window size
			GetClientRect ( hwnd, &size );

            Application* app = (Application*) (GetWindowLongPtr ( hwnd, GWLP_USERDATA ));

            uint32_t windowX = size.left;
            uint32_t windowY = size.top;
             
            uint32_t windowWidth = size.right - size.left;
            uint32_t windowHeight = size.bottom - size.top;

            app->platform->window->width = windowWidth;
            app->platform->window->height = windowHeight;

            WindowResizeEvent evt = WindowResizeEvent ();
            evt.app = app;
            evt.dimensions.x = windowWidth;
            evt.dimensions.y = windowHeight;

            app->gameEventSystem.Trigger ( evt );
			return 0;


		}

		// INPUT events
		// todo : look what is SYSKEY message
		case WM_KEYUP:
		{
			Application* app = (Application*) (GetWindowLongPtr ( hwnd, GWLP_USERDATA ));
			app->platform->input->SetKeyUp ( (KeyCode) wParam );
			return 0;
		}

		case WM_KEYDOWN:
		{
			// use of lookup to avoid cache misses
			size_t isPressedAslookupIndex = (size_t) ((lParam & (1 << 30)) == 0);

			Win32Utils::keydownFuncLookup[isPressedAslookupIndex] ( hwnd, wParam );

			return 0;
		}

		case WM_MOUSEMOVE:
		{
			uint32_t x = GET_X_LPARAM ( lParam );
			uint32_t y = GET_Y_LPARAM ( lParam );

			Vector2Int pos;
			pos.x = x;
			pos.y = y;



			Application* app = (Application*) GetWindowLongPtr ( hwnd, GWLP_USERDATA );
			
            app->platform->input->SetMousePosition ( pos );

			return 1;
		}

		case WM_MOUSEWHEEL:
		{
			short delta = GET_WHEEL_DELTA_WPARAM ( wParam );

			size_t scrollDeltaLookupIndex = (size_t) (delta > 0);

			Win32Utils::scrollWheelLookup[scrollDeltaLookupIndex] ( hwnd, wParam );
			return 0;
		}

		case WM_LBUTTONDOWN:
		{
			Application* app = (Application*) (GetWindowLongPtr ( hwnd, GWLP_USERDATA ));
			app->platform->input->SetMouseDown ( MouseButton::LeftMouseButton );

			return 0;
		}
		case WM_MBUTTONDOWN:
		{
			Application* app = (Application*) (GetWindowLongPtr ( hwnd, GWLP_USERDATA ));
			app->platform->input->SetMouseDown ( MouseButton::MiddleMouseButton );

			return 0;
		}
		case WM_RBUTTONDOWN:
		{
			Application* app = (Application*) (GetWindowLongPtr ( hwnd, GWLP_USERDATA ));
			app->platform->input->SetMouseDown ( MouseButton::RightMouseButton );

			return 0;
		}
		case WM_LBUTTONUP:
		{
			Application* app = (Application*) (GetWindowLongPtr ( hwnd, GWLP_USERDATA ));
			app->platform->input->SetMouseUp ( MouseButton::LeftMouseButton );

			return 0;
		}
		case WM_MBUTTONUP:
		{
			Application* app = (Application*) (GetWindowLongPtr ( hwnd, GWLP_USERDATA ));
			app->platform->input->SetMouseUp ( MouseButton::MiddleMouseButton );

			return 0;
		}
		case WM_RBUTTONUP:
		{
			Application* app = (Application*) (GetWindowLongPtr ( hwnd, GWLP_USERDATA ));
			app->platform->input->SetMouseUp ( MouseButton::RightMouseButton );
			return 0;
		}
		default:
		{
			return DefWindowProc ( hwnd, uMsg, wParam, lParam );
		}
	}

}

void Win32Window::Startup ()
{
	Win32Utils::keydownFuncLookup[0] = Win32Utils::NoOp;
	Win32Utils::keydownFuncLookup[1] = Win32Utils::Keydown;
	
	Win32Utils::scrollWheelLookup[0] = Win32Utils::ScrollWheelNegative;
	Win32Utils::scrollWheelLookup[1] = Win32Utils::ScrollWheelPositive;


	WNDCLASS windowClass = { 0 };
	GameApp* gameApp = this->platform->app->gameApp;
	std::string name = gameApp->GetName ();
	std::wstring appNameWString = std::wstring ( name.begin (), name.end () );

	const wchar_t* appName = appNameWString.data ();

	windowClass.lpfnWndProc = HandleMessage;
	windowClass.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
	windowClass.hInstance = platform->processHandle;
	windowClass.lpszClassName = appName;
	windowClass.hbrBackground = CreateSolidBrush ( RGB ( 255, 0, 255 ) );

	RegisterClass ( &windowClass );

	// window styling
	long windowStyle = WS_SYSMENU | WS_CAPTION | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
	int  windowExStyle = WS_EX_ACCEPTFILES | WS_EX_APPWINDOW;

	uint32_t clientX = gameApp->gameStartup.x;
	uint32_t clientY = gameApp->gameStartup.y;
	uint32_t clientWidth = gameApp->gameStartup.width;
	uint32_t clientHeight = gameApp->gameStartup.height;


	uint32_t windowX = gameApp->gameStartup.x;
	uint32_t windowY = gameApp->gameStartup.y;
	uint32_t windowWidth = gameApp->gameStartup.width;
	uint32_t windowHeight = gameApp->gameStartup.height;
	RECT borderRect = { 0 };

	// this will get the window rect details based on the styling info passed
	AdjustWindowRectEx ( &borderRect, windowStyle, 0, windowExStyle );

	// in this case the "left" and "top" are negative , so we move the window pos by that amount
	windowX += borderRect.left;
	windowY += borderRect.top;

	windowWidth += borderRect.right - borderRect.left;
	windowHeight += borderRect.bottom - borderRect.top;

	windowHandle = CreateWindowEx (
		windowExStyle, // optional style
		appName, // class name
		appName, // window text
		windowStyle, // window style
		windowX, windowY, windowWidth, windowHeight,//position and size
		NULL,// parent wnd
		NULL,// Menu
		this->platform->processHandle, // app process
		NULL // additional app data
	);


	// store reference to the Win32 window as userData
	SetWindowLongPtr ( windowHandle, GWLP_USERDATA, (LONG_PTR) app );

	auto errorCode = GetLastError ();

	if ( windowHandle == NULL )
	{
		return;
	}

	if ( ShowWindow ( windowHandle, SW_SHOW ) )
	{
		MessageBox ( NULL, TEXT ( "Showing of the window failed" ), TEXT ( "Error" ), MB_OK );
		return;
	}

	app->gameEventSystem.Listen<EngineCloseEvent> ( Action<EngineCloseEvent> ( HandleEngineClose ) );
}



void Win32Window::Destroy ()
{
	app->gameEventSystem.Unlisten<EngineCloseEvent> ( Action<EngineCloseEvent> ( HandleEngineClose ) );
}
