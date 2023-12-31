#include "GameEventSystem.h"
#include "Types/GameEvents.h"
#include <Allocators/Allocator.h>
#include "../Global/Global.h"

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

    WindowFocusEvent::GetID ();
    WindowUnfocusEvent::GetID ();
    WindowResizeEvent::GetID ();

	this->eventListenersCount = IDProvider<Event>::CurrentCounter ();

    Allocator heap_alloc = Global::alloc_toolbox.heap_allocator;

    DArray<DArray<void*>>::Create(this->eventListenersCount, &this->eventListenersCallbacks, heap_alloc);

	for ( int i = 0; i < this->eventListenersCount; ++i )
	{
        DArray<void*> callback_list;
        DArray<void*>::Create(0, &callback_list, heap_alloc);
		eventListenersCallbacks.data[i] = callback_list;
	}
}

void GameEventSystem::Destroy ()
{}
