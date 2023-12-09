#include "InputUtils.h"
#include "../../Application/Application.h"
#include "../../EventSystem/GameEventSystem.h"
#include "../../EventSystem/Types/GameEvents.h"
#include "../../Global/Global.h"

ActionParams<Input*, KeyCode> InputUtils::keyboadCallbacksLookup[4];
ActionParams<Input*, MouseButton> InputUtils::mouseCallbacksLookup[4];

void InputUtils::KeyNoOpProc ( Input* input, KeyCode keycode )
{}

void InputUtils::KeyDownProc ( Input* input, KeyCode keycode )
{
    Global::event_system.Trigger<KeyDownEvent> ( KeyDownEvent () );
}

void InputUtils::KeyPressProc ( Input* input, KeyCode keycode )
{
    Global::event_system.Trigger<KeyPressedEvent> ( KeyPressedEvent () );
}

void InputUtils::KeyUpProc ( Input* input, KeyCode keycode )
{
    Global::event_system.Trigger<KeyUpEvent> ( KeyUpEvent () );
}

void InputUtils::MouseNoOpProc ( Input* input, MouseButton mouseButton )
{}

void InputUtils::MouseDownProc ( Input* input, MouseButton mouseButton )
{
    Global::event_system.Trigger<MouseButtonDownEvent> ( MouseButtonDownEvent () );
}

void InputUtils::MousePressProc ( Input* input, MouseButton mouseButton )
{
    Global::event_system.Trigger<MouseButtonPressedEvent> ( MouseButtonPressedEvent () );
}

void InputUtils::MouseUpProc ( Input* input, MouseButton mouseButton )
{
    Global::event_system.Trigger<MouseButtonUpEvent> ( MouseButtonUpEvent () );
}

