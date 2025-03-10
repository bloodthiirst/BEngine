#pragma once
#include "../Allocators/Allocator.h"

template<typename T>
struct LinkedList
{
    struct Node
    {
        Node* next;
        T data;
    };

    size_t size;
    Node* first;
    Node* last;
    Allocator alloc;

    static void Create(LinkedList* out_list, Allocator alloc)
    {
        *out_list = {};
        out_list->alloc = alloc;
    }

    static void Destroy(LinkedList* out_list)
    {
        Node* n = out_list->first;

        while(n != nullptr)
        {
            Node* next = n->next;
            FREE(out_list->alloc , n);
            n = next;
        }

        *out_list = {};
    }

    static void RemoveAt(LinkedList* in_list , size_t index)
    {
        assert( index < in_list->size && index > -1);

        Node* n = in_list->first;
        
        while( size_t i = 0; i != index)
        {
            n = n->next;
        }

        Node* next = n->next;
        prev->next = next;
        FREE(in_list->alloc , n);

        in_list->size--;
    }

    static void Append(LinkedList* in_list , T value)
    {  
        Node* new_node = (Node*) ALLOC(in_list->alloc ,sizeof(Node));
        *new_node = {};
        new_node->data = value;

        in_list->size++;

        if(in_list->last == nullptr)
        {
            in_list->first = new_node;
            in_list->last = new_node;
            return;
        }

        Node* last_node = in_list->last;
        new_node->prev = last_node;
        last_node->next = new_node;
        in_list->last = new_node;
    }
};
