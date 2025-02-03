#pragma once
#include "DArray.h"
#include "../Allocators/Allocator.h"


template<typename T>
struct LinkedListNode
{
    LinkedListNode* prev;
    LinkedListNode* next;
    T data;
};


template<typename T>
struct LinkedList
{
    size_t size;
    LinkedListNode<T>* first;
    LinkedListNode<T>* last;
    Allocator alloc;

    static void Create(LinkedList* out_list, Allocator alloc)
    {
        *out_list = {};
        out_list->alloc = alloc;
    }

    static void Destroy(LinkedList* out_list)
    {
        LinkedListNode<T>* n = out_list->first;

        while(n != nullptr)
        {
            LinkedListNode<T>* next = n->next;
            out_list->alloc.free(&out_list->alloc , n);
            n = next;
        }

        *out_list = {};
    }

    static void RemoveAt(LinkedList* in_list , size_t index)
    {
        assert( index < in_list->size && index > -1);

        LinkedListNode<T>* n = in_list->first;
        
        while( size_t i = 0; i != index)
        {
            n = n->next;
        }

        LinkedListNode<T>* prev = n->prev;
        LinkedListNode<T>* next = n->next;
        prev->next = next;
        next->prev = prev;

        in_list->alloc.free(&in_list->alloc , n);

        in_list->size--;
    }

    static void Append(LinkedList* in_list , T value)
    {  
        LinkedListNode<T>* new_node = (LinkedListNode<T>*) in_list->alloc.alloc(&in_list->alloc , sizeof(LinkedListNode<T>));
        *new_node = {};
        new_node->data = value;

        in_list->size++;

        if(in_list->last == nullptr)
        {
            in_list->first = new_node;
            in_list->last = new_node;
            return;
        }

        LinkedListNode<T>* last_node = in_list->last;
        new_node->prev = last_node;
        last_node->next = new_node;
        in_list->last = new_node;
    }
};
