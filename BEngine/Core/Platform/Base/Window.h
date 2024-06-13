#pragma once
#include <stdint.h>
#include <Typedefs/Typedefs.h>
#include "../../Application/Application.h"

struct Platform;

struct Window
{
    /// <summary>
    /// A pointer to a custom user-defined struct for extra state
    /// </summary>
    void* user_data;

    /// <summary>
    /// Window width
    /// </summary>
    uint32_t width;

    /// <summary>
    /// Window height
    /// </summary>
    uint32_t height;

    /// <summary>
    /// Is the window currently focused ?
    /// </summary>
    bool is_focused;

    /// <summary>
    /// Handles the startup of the process based of the game setup provided by the app
    /// </summary>
    /// <returns></returns>
    ActionParams<Window* , ApplicationStartup> startup_callback;

    /// <summary>
    /// Cleanup before killing the process
    /// </summary>
    /// <returns></returns>
    Action destroy;

    /// <summary>
    /// <para>Handles the messages pushed by the OS</para>
    /// <para>Returns false when the receiving the QUIT message from the OS</para>
    /// </summary>
    /// <returns></returns>
    Func<bool> handle_messages;
};

