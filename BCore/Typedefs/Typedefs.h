#pragma once

template<typename T>
struct DArray;

using Action = void(*)();

template <typename... Args>
using ActionParams = void(*)(Args...);

template <typename TReturn , typename... Args>
using Func = TReturn(*)(Args...);

template <typename... Args>
using EventArray = DArray<ActionParams<Args>>;