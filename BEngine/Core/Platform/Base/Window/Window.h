#pragma once
#include <windows.h>
#include <GameApp.h>

class Application;

class Window
{
public:

	Application* app = nullptr;

    uint32_t width;
    uint32_t height;

	/// <summary>
	/// Handles the startup of the process based of the game setup provided by the app
	/// </summary>
	/// <returns></returns>
	virtual void Startup () = 0;

	/// <summary>
	/// Cleanup before killing the process
	/// </summary>
	/// <returns></returns>
	virtual void Destroy () = 0;

	/// <summary>
	/// <para>Handles the messages pushed by the OS</para>
	/// <para>Returns false when the receiving the QUIT message from the OS</para>
	/// </summary>
	/// <returns></returns>
	virtual bool HandleMessages () = 0;
};

