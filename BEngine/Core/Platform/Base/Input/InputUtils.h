#pragma once
#include "Input.h"

typedef void (KeyboardEventFuncPtr) ( Input*, KeyCode );
typedef void (MouseEventFuncPtr) ( Input*, MouseButton );
class InputUtils
{

public:

	/// <summary>
	/// Lookup table used to jump directly to the correct event invokation to reduce cache misses
	/// </summary>
	static KeyboardEventFuncPtr* keyboadCallbacksLookup[4];
	static MouseEventFuncPtr* mouseCallbacksLookup[4];

public:
	static void KeyNoOpProc ( Input* input, KeyCode keycode );

	static void KeyDownProc ( Input* input, KeyCode keycode );

	static void KeyPressProc ( Input* input, KeyCode keycode );

	static void KeyUpProc ( Input* input, KeyCode keycode );

	static void MouseNoOpProc ( Input* input, MouseButton mouseButton );

	static void MouseDownProc ( Input* input, MouseButton mouseButton );

	static void MousePressProc ( Input* input, MouseButton mouseButton );

	static void MouseUpProc ( Input* input, MouseButton mouseButton );
};