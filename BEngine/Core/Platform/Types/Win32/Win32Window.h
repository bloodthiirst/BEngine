#pragma once
#include <windows.h>
#include <windowsx.h>
#include <Allocators/Allocator.h>
#include <Maths/Vector2Int.h>
#include <GameApp.h>
#include "Win32Utils.h"
#include "../../Base/Input.h"
#include "../../Base/Window.h"
#include "../../../EventSystem/GameEventSystem.h"
#include "../../../Global/Global.h"
#include "../../../EventSystem/Types/GameEvents.h"
#include "../../../Application/Application.h"

struct Win32WindowState
{
    HWND window_handle;
    HINSTANCE process_handle;
};

struct Win32Window
{

    static void Create( Window* window )
    {
        window->handle_messages = HandleMessages;
        window->startup_callback = Startup;
        window->destroy = Destroy;
    }

    static void Startup( Window* window, ApplicationStartup startup )
    {
        Win32WindowState* state = Global::alloc_toolbox.HeapAlloc<Win32WindowState>();
        window->user_data = state;

        state->window_handle = nullptr;
        state->process_handle = GetModuleHandle( nullptr );

        Win32Utils::keydownFuncLookup[0] = Win32Utils::NoOp;
        Win32Utils::keydownFuncLookup[1] = Win32Utils::Keydown;

        Win32Utils::scrollWheelLookup[0] = Win32Utils::ScrollWheelNegative;
        Win32Utils::scrollWheelLookup[1] = Win32Utils::ScrollWheelPositive;

        WNDCLASS windowClass = { 0 };
        GameApp game_app = Global::app.game_app;
        StringView name = Defines::engine_name;

        Allocator heap_alloc = Global::alloc_toolbox.heap_allocator;

        windowClass.lpfnWndProc = HandleMessage;
        windowClass.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
        windowClass.hInstance = state->process_handle;
        windowClass.lpszClassName = name.buffer;
        windowClass.hbrBackground = CreateSolidBrush( RGB( 255, 0, 255 ) );
        windowClass.hCursor = LoadCursor( NULL, IDC_ARROW );
        RegisterClass( &windowClass );

        // window styling
        int64_t windowStyle = WS_SYSMENU | WS_CAPTION | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
        int32_t  windowExStyle = WS_EX_ACCEPTFILES | WS_EX_APPWINDOW;

        uint32_t clientX = (uint32_t) startup.window_rect.x;
        uint32_t clientY = (uint32_t) startup.window_rect.y;
        uint32_t clientWidth = (uint32_t) startup.window_rect.width;
        uint32_t clientHeight = (uint32_t) startup.window_rect.height;


        uint32_t windowX = (uint32_t) startup.window_rect.x;
        uint32_t windowY = (uint32_t) startup.window_rect.y;
        uint32_t windowWidth = (uint32_t) startup.window_rect.width;
        uint32_t windowHeight = (uint32_t) startup.window_rect.height;
        RECT borderRect = { 0 };

        // this will get the window rect details based on the styling info passed
        AdjustWindowRectEx( &borderRect, (DWORD) windowStyle, 0, (DWORD) windowExStyle );

        // in this case the "left" and "top" are negative , so we move the window pos by that amount
        windowX += borderRect.left;
        windowY += borderRect.top;

        windowWidth += borderRect.right - borderRect.left;
        windowHeight += borderRect.bottom - borderRect.top;

        state->window_handle = CreateWindowEx(
            windowExStyle, // optional style
            name.buffer, // class name
            name.buffer, // window text
            (DWORD) windowStyle, // window style
            windowX, windowY, windowWidth, windowHeight,//position and size
            nullptr,// parent wnd
            nullptr,// Menu
            state->process_handle, // app process
            nullptr // additional app data
        );


        // store reference to the Win32 window as userData
        //SetWindowLongPtr(state->window_handle, GWLP_USERDATA, (LONG_PTR)app);

        auto errorCode = GetLastError();

        if ( state->window_handle == NULL )
        {
            return;
        }

        if ( ShowWindow( state->window_handle, SW_SHOW ) )
        {
            MessageBox( nullptr, TEXT( "Showing of the window failed" ), TEXT( "Error" ), MB_OK );
            return;
        }

        Global::event_system.Listen<EngineCloseEvent>( HandleEngineClose );
    }


    static bool HandleMessages()
    {
        MSG msg = { 0 };
        bool hasQuit = false;

        while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ))
        {
            TranslateMessage( &msg );

            DispatchMessage( &msg );

            hasQuit |= (msg.message == WM_QUIT);
        }

        return !hasQuit;
    }

    static void HandleEngineClose( EngineCloseEvent evt )
    {
        Win32WindowState* state = (Win32WindowState*) Global::platform.window.user_data;
        HWND hwnd = state->window_handle;
        DestroyWindow( hwnd );
    }


    /// <summary>
    /// Handle the OS messages
    /// </summary>
    /// <param name="hwnd"></param>
    /// <param name="uMsg"></param>
    /// <param name="wParam"></param>
    /// <param name="lParam"></param>
    /// <returns>returning 1 means the message has been handled</returns>
    static LRESULT CALLBACK HandleMessage( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
    {
        switch ( uMsg )
        {
        case WM_DESTROY:
        {
            PostQuitMessage( 0 );
            return 0;
        }
        case WM_CLOSE:
        {
            if ( MessageBox( hwnd, TEXT( "Are you sure you want to exit?" ), TEXT( "Exit" ), MB_YESNO | MB_ICONQUESTION ) == IDYES )
            {
                EngineCloseEvent evt = EngineCloseEvent();

                Global::event_system.Trigger<EngineCloseEvent>( evt );
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

        case WM_SETFOCUS:
        {
            Global::app.application_state.is_focused = true;

            WindowFocusEvent evt = WindowFocusEvent();

            Global::event_system.Trigger<WindowFocusEvent>( evt );

            return 0;
        }
        case WM_KILLFOCUS:
        {
            Global::app.application_state.is_focused = false;

            WindowUnfocusEvent evt = WindowUnfocusEvent();

            Global::event_system.Trigger<WindowUnfocusEvent>( evt );

            return 0;
        }
        case WM_SIZE:
        {
            RECT size;

            // get the new window size
            GetClientRect( hwnd, &size );

            uint32_t windowX = size.left;
            uint32_t windowY = size.top;

            uint32_t windowWidth = size.right - size.left;
            uint32_t windowHeight = size.bottom - size.top;

            Global::platform.window.width = windowWidth;
            Global::platform.window.height = windowHeight;

            WindowResizeEvent evt = WindowResizeEvent();
            evt.dimensions.x = windowWidth;
            evt.dimensions.y = windowHeight;

            Global::event_system.Trigger( evt );
            return 0;
        }

        // INPUT events
        // todo : look what is SYSKEY message
        case WM_KEYUP:
        {
            Global::platform.input.SetKeyUp( (KeyCode) wParam );
            return 0;
        }

        case WM_KEYDOWN:
        {
            // use of lookup to avoid cache misses
            size_t isPressedAslookupIndex = (size_t) ((lParam & (1 << 30)) == 0);

            Win32Utils::keydownFuncLookup[isPressedAslookupIndex]( hwnd, wParam );

            return 0;
        }

        case WM_MOUSEMOVE:
        {
            uint32_t x = GET_X_LPARAM( lParam );
            uint32_t y = GET_Y_LPARAM( lParam );

            Vector2Int pos;
            pos.x = x;
            pos.y = y;

            Global::platform.input.SetMousePosition( pos );

            return 0;
        }

        case WM_MOUSEWHEEL:
        {
            int16_t delta = GET_WHEEL_DELTA_WPARAM( wParam );

            size_t scrollDeltaLookupIndex = (size_t) (delta > 0);

            Win32Utils::scrollWheelLookup[scrollDeltaLookupIndex]( hwnd, wParam );
            return 0;
        }

        case WM_LBUTTONDOWN:
        {
            SetCapture( hwnd );
            Global::platform.input.SetMouseDown( MouseButton::LeftMouseButton );

            return 0;
        }
        case WM_MBUTTONDOWN:
        {
            SetCapture( hwnd );
            Global::platform.input.SetMouseDown( MouseButton::MiddleMouseButton );

            return 0;
        }
        case WM_RBUTTONDOWN:
        {
            SetCapture( hwnd );
            Global::platform.input.SetMouseDown( MouseButton::RightMouseButton );

            return 0;
        }
        case WM_LBUTTONUP:
        {
            ReleaseCapture();
            Global::platform.input.SetMouseUp( MouseButton::LeftMouseButton );

            return 0;
        }
        case WM_MBUTTONUP:
        {
            ReleaseCapture();
            Global::platform.input.SetMouseUp( MouseButton::MiddleMouseButton );

            return 0;
        }
        case WM_RBUTTONUP:
        {
            ReleaseCapture();
            Global::platform.input.SetMouseUp( MouseButton::RightMouseButton );
            return 0;
        }
        default:
        {
            return DefWindowProc( hwnd, uMsg, wParam, lParam );
        }
        }
    }
private :
    static void Destroy()
    {
        Global::event_system.Unlisten<EngineCloseEvent>( HandleEngineClose );
    }
};