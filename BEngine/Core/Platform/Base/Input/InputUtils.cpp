#include "InputUtils.h"
#include "../../../Application/Application.h"

KeyboardEventFuncPtr* InputUtils::keyboadCallbacksLookup[4];
MouseEventFuncPtr* InputUtils::mouseCallbacksLookup[4];


void InputUtils::KeyNoOpProc ( Input* input, KeyCode keycode )
{}

void InputUtils::KeyDownProc ( Input* input, KeyCode keycode )
{
	input->app->gameEventSystem.Trigger<KeyDownEvent> ( KeyDownEvent () );
}

void InputUtils::KeyPressProc ( Input* input, KeyCode keycode )
{
	input->app->gameEventSystem.Trigger<KeyPressedEvent> ( KeyPressedEvent () );
}

void InputUtils::KeyUpProc ( Input* input, KeyCode keycode )
{
	input->app->gameEventSystem.Trigger<KeyUpEvent> ( KeyUpEvent () );
}

void InputUtils::MouseNoOpProc ( Input* input, MouseButton mouseButton )
{}

void InputUtils::MouseDownProc ( Input* input, MouseButton mouseButton )
{
	input->app->gameEventSystem.Trigger<MouseButtonDownEvent> ( MouseButtonDownEvent () );
}

void InputUtils::MousePressProc ( Input* input, MouseButton mouseButton )
{
	input->app->gameEventSystem.Trigger<MouseButtonPressedEvent> ( MouseButtonPressedEvent () );
}

void InputUtils::MouseUpProc ( Input* input, MouseButton mouseButton )
{
	input->app->gameEventSystem.Trigger<MouseButtonUpEvent> ( MouseButtonUpEvent () );
}

