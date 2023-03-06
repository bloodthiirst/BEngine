#include "GameEventSystem.h"

void GameEventSystem::Startup ()
{
	// initialize the IDs for the gameEvents

	GameStartEvent::GetID();
	GameEndEvent::GetID();
	
	KeyDownEvent::GetID();
	KeyPressedEvent::GetID();
	KeyUpEvent::GetID();

	MouseButtonDownEvent::GetID ();
	MouseButtonPressedEvent::GetID ();
	MouseButtonUpEvent::GetID ();

	EngineCloseEvent::GetID();

    WindowResizeEvent::GetID ();

	this->eventListenersCount = IDProvider<Event>::CurrentCounter ();

	this->eventListenersCallbacks = new std::vector<ActionBase*>*[eventListenersCount];

	for ( int i = 0; i < this->eventListenersCount; ++i )
	{
		auto callbackList = new std::vector<ActionBase*> ();
		eventListenersCallbacks[i] = callbackList;
	}
}

void GameEventSystem::Destroy ()
{}
