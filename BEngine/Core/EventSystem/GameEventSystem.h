#pragma once
#include "Types/GameEvents.h"
#include "../Logger/Logger.h"
#include <typeinfo>
#include <functional>
#include  <list>
#include "Action.h"

template<typename TEvent>
using CallbackFunctionPtr = void (*)(TEvent);


class GameEventSystem
{
private:
	std::vector<ActionBase*>** eventListenersCallbacks;
	size_t eventListenersCount;
public:
	void Startup ();
	void Destroy ();

	template<typename TEvent>
	void Trigger ( TEvent eventData )
	{
		size_t index = IDCache<TEvent>::id;

		std::vector<ActionBase*>* vector = eventListenersCallbacks[index];

		std::string logText = std::string ( "Event::" );
		logText += typeid(TEvent).name ();

		const char* cStr = logText.c_str ();

		Logger::Log ( cStr );

		for ( auto& it : *vector )
		{
			Action<TEvent>* functionPtr = (Action<TEvent>*) (it);

			functionPtr->Invoke ( eventData );
		}
	}


	template<typename TEvent>
	void Listen ( Action<TEvent>  callback )
	{
		size_t index = IDCache<TEvent>::id;
		eventListenersCallbacks[index]->push_back ( new Action<TEvent> ( callback ) );
	}


	template<typename TEvent>
	void Unlisten ( Action<TEvent> callback )
	{
		size_t index = IDCache<TEvent>::id;

		std::vector<ActionBase*> vector = (*eventListenersCallbacks[index]);

		for ( int i = 0; i < vector.size (); ++i )
		{
			ActionBase* curr = vector[i];

			if ( curr->funcPtr == callback.funcPtr && curr->instance == callback.instance )
			{
				vector.erase ( vector.begin () + i );
				return;
			}
		}
	}

};



