#pragma once
#include "KeyCode.h"
#include "../../../EventSystem/GameEventSystem.h"
#include "../../../Maths/Vector2Int.h"
#include <memory>

class Application;

enum MouseButton
{
	LeftMouseButton,
	RightMouseButton,
	MiddleMouseButton,

	MAX_MOUSE_BUTTONS
};

enum InputKeyState
{
	CurrentValue	= 0b01,
	PreviousValue	= 0b10,
};


/// <summary>
/// <para>A 1 byte used to track/represent the state of a key</para>
/// <para>Each bit represent the state of the key across the last 8 frames ( 1 byte = 8 bits so 8 frames in total) </para>
/// <para>Bit 0 represents the current frame [t], Bit 1 represents the frame [t - 1] , Bit 2 represents the frame [t - 2] , etc ...</para>
/// </summary>
typedef char KeyState;

struct KeyboardState
{
	KeyState inputKeyStates[KeyCode::MAX_KEYBOARD_KEYS];
};

struct MouseState
{
	KeyState mouseKeyStates[MouseButton::MAX_MOUSE_BUTTONS];
	Vector2Int currentMousePosition;
	Vector2Int previousMousePosition;
	int8_t currentScrollWheelDelta;
	int8_t previousScrollWheelDelta;
};

class Input
{
public:
	Application* app;

public:
	KeyboardState keyboardState;
	MouseState mouseState;	

public:
	void Startup ();
	void Destroy ();

	/// <summary>
	/// Called before the frame update and render mainly to trigger the keyup/keydown events
	/// </summary>
	/// <param name="deltaTime"></param>
	void OnUpdate ( float deltaTime );

	/// <summary>
	/// Called after the frame update and render to cache the previous state
	/// </summary>
	/// <param name="deltaTime"></param>
	void OnPostUpdate ( float deltaTime );

	/// <summary>
	/// Is keyboard key pressed ?
	/// </summary>
	bool IsPressed ( KeyCode keyCode );

	/// <summary>
	/// Is keyboard key up ?
	/// </summary>
	bool IsUp ( KeyCode keyCode );

	/// <summary>
	/// Is keyboard key down ?
	/// </summary>
	bool IsDown ( KeyCode keyCode );

	/// <summary>
	/// Is mouse key pressed ?
	/// </summary>
	bool IsPressed ( MouseButton moueButton );

	/// <summary>
	/// Is mouse key up ?
	/// </summary>
	bool IsUp ( MouseButton moueButton );

	/// <summary>
	/// Is mouse key down ?
	/// </summary>
	bool IsDown ( MouseButton moueButton );

	Vector2Int GetMousePosition ();

	void SetKeyDown ( KeyCode keyCode );
	void SetKeyUp ( KeyCode keyCode );

	void SetMouseDown ( MouseButton mouseButton);
	void SetMouseUp ( MouseButton mouseButton);

	void SetScrollWheel ( std::int8_t scrollDelta );

	void SetMousePosition ( Vector2Int pos );

};

