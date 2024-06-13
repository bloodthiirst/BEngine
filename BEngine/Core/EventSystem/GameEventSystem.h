#pragma once
#include <typeinfo>
#include <Typedefs/Typedefs.h>
#include <String/StringBuffer.h>
#include <String/StringView.h>
#include <String/StringUtils.h>
#include <Allocators/Allocator.h>
#include <Containers/DArray.h>
#include "../Logger/Logger.h"
#include "Base/EventBase.h"

class GameEventSystem
{
private:
    DArray<DArray<void*>> eventListenersCallbacks;
    size_t eventListenersCount;
public:
    void Startup();
    void Destroy();

    template<typename TEvent>
    void Trigger(TEvent eventData)
    {
        size_t index = IDCache<TEvent>::id;

        DArray<void*> precast = eventListenersCallbacks.data[index];
        DArray<ActionParams<TEvent>> callbacksArr =  *(DArray<ActionParams<TEvent>>*)((void*) & precast);

        Allocator heap_alloc = HeapAllocator::Create();

        StringView prefix = StringView::Create("Event::");
        StringView type_name = StringView::Create(typeid(TEvent).name());
        StringBuffer logText = StringUtils::Concat( heap_alloc , prefix, type_name);

        char* c_str = StringView::ToCString(logText.view, heap_alloc);
        //Global::logger.Log(c_str);

        for (size_t i = 0; i < callbacksArr.size; ++i)
        {
            ActionParams<TEvent> functionPtr = callbacksArr.data[i];

            functionPtr(eventData);
        }

        heap_alloc.free(&heap_alloc, logText.buffer);
        heap_alloc.free(&heap_alloc, c_str);
    }

    template<typename TEvent>
    void Listen(ActionParams<TEvent>  callback)
    {
        size_t index = IDCache<TEvent>::id;

        DArray<ActionParams<TEvent>>* callbacksArr = (DArray<ActionParams<TEvent>>*) & eventListenersCallbacks.data[index];
        DArray<ActionParams<TEvent>>::Add(callbacksArr, callback);
    }

    template<typename TEvent>
    void Unlisten(ActionParams<TEvent> callback)
    {
        size_t index = IDCache<TEvent>::id;

        Allocator stack_alloc = HeapAllocator::Create();

        DArray<void*> precast = eventListenersCallbacks.data[index];
        DArray<ActionParams<TEvent>> callbacksArr = *(DArray<ActionParams<TEvent>>*)((void*)&precast);

        for (int i = 0; i < callbacksArr.size; ++i)
        {
            ActionParams<TEvent> curr = callbacksArr.data[i];

            if (curr == callback)
            {
                DArray<ActionParams<TEvent>>::RemoveAll(&callbacksArr, curr);
                return;
            }
        }
    }
};