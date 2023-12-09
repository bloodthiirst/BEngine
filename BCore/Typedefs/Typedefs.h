#pragma once
using Action = void(*)();

template <typename... Args>
using ActionParams = void(*)(Args...);

template <typename TReturn , typename... Args>
using Func = TReturn(*)(Args...);