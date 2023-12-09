#pragma once
#include <Maths/Vector2Int.h>
#include <memory>
#include "../../EventSystem/GameEventSystem.h"
#include "../../Utils/MemoryUtils.h"
#include "KeyCode.h"
#include "InputUtils.h"

enum MouseButton
{
    LeftMouseButton,
    RightMouseButton,
    MiddleMouseButton,

    MAX_MOUSE_BUTTONS
};

enum InputKeyState
{
    CurrentValue = 0b01,
    PreviousValue = 0b10,
};

/// <summary>
/// <para>KeyState is a 1 byte struct used to track/represent the state of a key</para>
/// <para>Each bit represent the state of the key across the last 8 frames ( 1 byte = 8 bits so 8 frames in total) </para>
/// <para>Bit 0 represents the current frame [t], Bit 1 represents the previous frame [t - 1] , Bit 2 represents the one before [t - 2] , etc ...</para>
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

struct Input
{
    KeyboardState keyboardState;
    MouseState mouseState;

    static void Create(Input* input)
    {
        InputUtils::keyboadCallbacksLookup[0] = InputUtils::KeyNoOpProc;
        InputUtils::keyboadCallbacksLookup[1] = InputUtils::KeyDownProc;
        InputUtils::keyboadCallbacksLookup[2] = InputUtils::KeyUpProc;
        InputUtils::keyboadCallbacksLookup[3] = InputUtils::KeyPressProc;

        InputUtils::mouseCallbacksLookup[0] = InputUtils::MouseNoOpProc;
        InputUtils::mouseCallbacksLookup[1] = InputUtils::MouseDownProc;
        InputUtils::mouseCallbacksLookup[2] = InputUtils::MouseUpProc;
        InputUtils::mouseCallbacksLookup[3] = InputUtils::MousePressProc;

        memset(&input->keyboardState.inputKeyStates, 0, sizeof(KeyState) * KeyCode::MAX_KEYBOARD_KEYS);
        memset(&input->mouseState.mouseKeyStates, 0, sizeof(KeyState) * MouseButton::MAX_MOUSE_BUTTONS);
    }


    void Destroy()
    {

    }


    void OnUpdate(float deltaTime)
    {
        // we only need the state from the current and previous frame to determine the up/down state of the key
        // so we use the bits as an index in a lookup table such as :
        // 00 -> key hasn't been pressed during the current frame and previous one
        // 01 -> key started being pressed during the current frame
        // 11 -> key was pressed during last frame and still is during this frame
        // 10 -> key just got released

        // keyboard
        {
            KeyState* start = &keyboardState.inputKeyStates[0];
            KeyState* end = start + KeyCode::MAX_KEYBOARD_KEYS;

            for (int i = 0; start != end; ++i, ++start)
            {
                KeyState value = *start & 0b0000011;
                InputUtils::keyboadCallbacksLookup[value](this, (KeyCode)i);
            }
        }

        // mouse
        {
            KeyState* start = &mouseState.mouseKeyStates[0];
            KeyState* end = start + MouseButton::MAX_MOUSE_BUTTONS;
            for (int i = 0; start != end; ++i, ++start)
            {
                KeyState value = *start & 0b0000011;
                InputUtils::mouseCallbacksLookup[value](this, (MouseButton)i);
            }
        }
    }


    void OnPostUpdate(float deltaTime)
    {
        // keyboard
        {
            size_t keyboardWidth = KeyCode::MAX_KEYBOARD_KEYS;

            char* start = this->keyboardState.inputKeyStates;
            char* end = start + keyboardWidth;


            for (char* p = start; p != end; ++p)
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

            for (char* p = start; p != end; ++p)
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

    bool IsPressed(KeyCode keyCode)
    {
        return keyboardState.inputKeyStates[keyCode] & InputKeyState::CurrentValue;
    }

    bool IsUp(KeyCode keyCode)
    {
        return keyboardState.inputKeyStates[keyCode] == (InputKeyState::PreviousValue);
    }

    bool IsDown(KeyCode keyCode)
    {
        return keyboardState.inputKeyStates[keyCode] == InputKeyState::CurrentValue;
    }

    bool IsPressed(MouseButton moueButton)
    {
        return mouseState.mouseKeyStates[moueButton] & InputKeyState::CurrentValue;;
    }
    bool IsUp(MouseButton moueButton)
    {
        return mouseState.mouseKeyStates[moueButton] == (InputKeyState::PreviousValue);
    }

    bool IsDown(MouseButton moueButton)
    {
        return mouseState.mouseKeyStates[moueButton] == InputKeyState::CurrentValue;
    }

    void SetKeyDown(KeyCode keyCode)
    {
        size_t index = (size_t)keyCode;
        this->keyboardState.inputKeyStates[index] |= 1;
    }

    void SetKeyUp(KeyCode keyCode)
    {
        size_t index = (size_t)keyCode;
        this->keyboardState.inputKeyStates[index] &= ~1;
    }

    void SetMouseDown(MouseButton mouseButton)
    {
        this->mouseState.mouseKeyStates[mouseButton] |= 1;
    }
    void SetMouseUp(MouseButton mouseButton)
    {
        this->mouseState.mouseKeyStates[mouseButton] &= ~1;
    }

    void SetScrollWheel(std::int8_t scrollDelta)
    {
        this->mouseState.currentScrollWheelDelta = scrollDelta;
    }

    Vector2Int GetMousePosition()
    {
        return this->mouseState.currentMousePosition;
    }

    void SetMousePosition(Vector2Int pos)
    {
        this->mouseState.currentMousePosition = pos;
    }
};
