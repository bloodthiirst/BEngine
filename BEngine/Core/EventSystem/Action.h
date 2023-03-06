#pragma once

class ActionBase
{
public:
	void* instance;
	void* funcPtr;
};

template <typename... Args>
class Action : public ActionBase
{
using CallbackType = void(*)(Args...);
public:
	Action (CallbackType funcPtr )
	{
		this->instance = nullptr;
		this->funcPtr = funcPtr;
	}

	virtual void Invoke ( Args ... args )
	{
		CallbackType casted = (CallbackType)this->funcPtr;
		casted ( args ...);
	}
};

template <class TInstance, class... Args>
class Action<void (TInstance::*)(Args...)> : public Action<Args ...>
{
	using CallbackType = void(TInstance::*)(Args...);
public:
	Action ( TInstance* instance, CallbackType funcPtr )
	{
		this->instance = (void*) instance;
		this->funcPtr = (void*) funcPtr;
	}

	virtual void Invoke ( Args... args ) override
	{
		CallbackType casted = (CallbackType) this->funcPtr;
		this->instance->*casted ( args... );
	}
};