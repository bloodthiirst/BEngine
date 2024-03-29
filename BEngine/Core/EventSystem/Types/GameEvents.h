#pragma once
#include "../Base/EventBase.h"
#include <Maths/Vector2Int.h>

class GameStartEvent : public EventBase<GameStartEvent>
{};

class GameEndEvent : public EventBase<GameEndEvent>
{};

class KeyUpEvent : public EventBase<KeyUpEvent>
{};

class KeyPressedEvent : public EventBase<KeyPressedEvent>
{};

class KeyDownEvent : public EventBase<KeyDownEvent>
{};

class MouseButtonUpEvent : public EventBase<MouseButtonUpEvent>
{};

class MouseButtonPressedEvent : public EventBase<MouseButtonPressedEvent>
{};

class MouseButtonDownEvent : public EventBase<MouseButtonDownEvent>
{};

class WindowResizeEvent : public EventBase<MouseButtonDownEvent>
{
public:
    Vector2Int dimensions;
};

class EngineCloseEvent : public EventBase<EngineCloseEvent>
{
public:
};

class WindowFocusEvent : public EventBase<WindowFocusEvent>
{
public:

};

class WindowUnfocusEvent : public EventBase<WindowUnfocusEvent>
{
public:
};
