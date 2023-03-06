#pragma once
#include "Input.h"
#include "InputUtils.h"
#include "../../../Utils/MemoryUtils.h"
#include "../../../Application/Application.h"

void Input::Startup ()
{
	InputUtils::keyboadCallbacksLookup[0] = InputUtils::KeyNoOpProc;
	InputUtils::keyboadCallbacksLookup[1] = InputUtils::KeyDownProc;
	InputUtils::keyboadCallbacksLookup[2] = InputUtils::KeyUpProc;
	InputUtils::keyboadCallbacksLookup[3] = InputUtils::KeyPressProc;

	InputUtils::mouseCallbacksLookup[0] = InputUtils::MouseNoOpProc;
	InputUtils::mouseCallbacksLookup[1] = InputUtils::MouseDownProc;
	InputUtils::mouseCallbacksLookup[2] = InputUtils::MouseUpProc;
	InputUtils::mouseCallbacksLookup[3] = InputUtils::MousePressProc;

	memset ( &this->keyboardState.inputKeyStates, 0, sizeof ( KeyState ) * KeyCode::MAX_KEYBOARD_KEYS );
	memset ( &this->mouseState.mouseKeyStates, 0, sizeof ( KeyState ) * MouseButton::MAX_MOUSE_BUTTONS );
}

void Input::Destroy ()
{

}


void Input::OnUpdate ( float deltaTime )
{
	// we only need the state from the current and previous frame to determine the state of the key
	// so we use the bits as an index in a lookup table such as :
	// 00 -> key hasn't been pressed during the current frame and previous one
	// 01 -> key started being pressed during the current frame
	// 11 -> key was pressed during last frame and still is during this frame
	// 10 -> key just got released

	// keyboard
	{
		KeyState* start = &keyboardState.inputKeyStates[0];
		KeyState* end = start + KeyCode::MAX_KEYBOARD_KEYS;

		for ( int i = 0; start != end; ++i, ++start )
		{
			KeyState value = *start & 0b0000011;

			InputUtils::keyboadCallbacksLookup[value] ( this, (KeyCode) i );
		}
	}

	// mouse
	{
		KeyState* start = &mouseState.mouseKeyStates[0];
		KeyState* end = start + MouseButton::MAX_MOUSE_BUTTONS;
		for ( int i = 0; start != end; ++i, ++start )
		{
			KeyState value = *start & 0b0000011;

			InputUtils::mouseCallbacksLookup[value] ( this, (MouseButton) i );
		}
	}
}


void Input::OnPostUpdate ( float deltaTime )
{


	// keyboard
	{
		size_t keyboardWidth = KeyCode::MAX_KEYBOARD_KEYS;

		char* start = this->keyboardState.inputKeyStates;
		char* end = start + keyboardWidth;


		for ( char* p = start; p != end; ++p )
		{
			char value = *p;
			char isPressed = value & 1;
			value = (value << 1) | isPressed;
			*p = value;
		}
	}

	// mouse
	{
		size_t mouseWidth = MouseButton::MAX_MOUSE_BUTTONS;

		char* start = this->mouseState.mouseKeyStates;
		char* end = start + mouseWidth;

		for ( char* p = start; p != end; ++p )
		{
			char value = *p;
			char isPressed = value & 1;
			value = (value << 1) | isPressed;
			*p = value;
		}
	}

	this->mouseState.currentScrollWheelDelta = 0;

	// assgin current state to previous state
	this->mouseState.previousMousePosition = this->mouseState.currentMousePosition;
	this->mouseState.previousScrollWheelDelta = this->mouseState.currentScrollWheelDelta;
}

bool Input::IsPressed ( KeyCode keyCode )
{
	return keyboardState.inputKeyStates[keyCode] & InputKeyState::CurrentValue;
}

bool Input::IsUp ( KeyCode keyCode )
{
	return keyboardState.inputKeyStates[keyCode] == (InputKeyState::PreviousValue);
}

bool Input::IsDown ( KeyCode keyCode )
{
	return keyboardState.inputKeyStates[keyCode] == InputKeyState::CurrentValue;
}

bool Input::IsPressed ( MouseButton moueButton )
{
	return mouseState.mouseKeyStates[moueButton] & InputKeyState::CurrentValue;;
}
bool Input::IsUp ( MouseButton moueButton )
{
	return mouseState.mouseKeyStates[moueButton] == (InputKeyState::PreviousValue);
}

bool Input::IsDown ( MouseButton moueButton )
{
	return mouseState.mouseKeyStates[moueButton] == InputKeyState::CurrentValue;
}

void Input::SetKeyDown ( KeyCode keyCode )
{
	size_t index = (size_t) keyCode;
	this->keyboardState.inputKeyStates[index] |= 1;
}

void Input::SetKeyUp ( KeyCode keyCode )
{
	size_t index = (size_t) keyCode;
	this->keyboardState.inputKeyStates[index] &= ~1;
}

void Input::SetMouseDown ( MouseButton mouseButton )
{
	this->mouseState.mouseKeyStates[mouseButton] |= 1;
}
void Input::SetMouseUp ( MouseButton mouseButton )
{
	this->mouseState.mouseKeyStates[mouseButton] &= ~1;
}

void Input::SetScrollWheel ( std::int8_t scrollDelta )
{
	this->mouseState.currentScrollWheelDelta = scrollDelta;
}


Vector2Int Input::GetMousePosition ()
{
	return this->mouseState.currentMousePosition;
}

void Input::SetMousePosition ( Vector2Int pos )
{
	this->mouseState.currentMousePosition = pos;
}