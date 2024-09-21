#pragma once
#include "../Allocators/Allocator.h"
#include "../Containers/LinkedList.h"
#include "StringBuffer.h"
#include "StringView.h"
#include "StringUtils.h"

struct StringBuilder
{
    size_t size;
    LinkedList<StringBuffer> list;
    Allocator alloc;

    static void Create(StringBuilder* out_builder, Allocator alloc )
    {
        *out_builder = {};
        out_builder->alloc = alloc;
        LinkedList<StringBuffer>::Create(&out_builder->list , alloc);
    }

    static void Append(StringBuilder* in_builder , StringView str)
    {
        in_builder->size += str.length;
        StringBuffer buffer = StringBuffer::Create(str , in_builder->alloc);
        LinkedList<StringBuffer>::Append(&in_builder->list , buffer);
    }

    static void ToString(StringBuilder* in_builder , StringBuffer* out_str , Allocator alloc)
    {
            *out_str = StringBuffer::Create(in_builder->size , alloc);

            LinkedListNode<StringBuffer>* n = in_builder->list.first;
            
            size_t offset = 0;

            while(n != nullptr)
            {
                CoreContext::mem_copy( (void*) n->data.buffer ,(void*) (out_str->buffer + offset) , n->data.length);
                offset += n->data.length;

                n = n->next;
            }
    }

    static void Destroy(StringBuilder* in_builder)
    {
        LinkedList<StringBuffer>::Destroy(&in_builder->list);
        *in_builder = {};
    }
};