#pragma once
#include "DArray.h"

struct FreeList
{
    struct Node
    {
        uint32_t start;
        uint32_t size;
    };

    /// <summary>
    /// Ordered list of all the used nodes
    /// </summary>
    DArray<Node> used_nodes;

    /// <summary>
    /// Ordered list of all the free nodes
    /// </summary>
    DArray<Node> free_nodes;

    /// <summary>
    /// Total memory being managed by the free list
    /// </summary>
    size_t total_mem;

    /// <summary>
    /// Currently used memory
    /// </summary>
    size_t used_mem;

    static void Create(FreeList *out_freelist, size_t nodes_capacity, size_t total_mem, Allocator alloc)
    {
        out_freelist->total_mem = total_mem;
        out_freelist->used_mem = 0;

        DArray<Node>::Create(nodes_capacity, &out_freelist->used_nodes, alloc);
        DArray<Node>::Create(nodes_capacity, &out_freelist->free_nodes, alloc);

        Node first = {};
        first.start = 0;
        first.size = (uint32_t)total_mem;

        DArray<Node>::Add(&out_freelist->free_nodes, first);
    }

    static void FreeBlock(FreeList *in_freelist, Node in_node)
    {

        // remove used block
        {
            size_t remove_index = {};
            FindRemoveIndex(&in_freelist->used_nodes, in_node, &remove_index);

            assert(remove_index != -1);

            in_freelist->used_mem -= in_node.size;

            DArray<Node>::RemoveAt(&in_freelist->used_nodes, remove_index);
        }

        // if no free nodes available
        if (in_freelist->free_nodes.size == 0)
        {
            DArray<Node>::Add(&in_freelist->free_nodes, in_node);
            return;
        }

        // add the free block
        size_t insert_index = {};
        FindInsertIndex(&in_freelist->free_nodes, in_node, &insert_index);

        assert(insert_index != -1);

        uint32_t start = in_node.start;
        uint32_t start_index = (uint32_t)insert_index;

        DArray<Node>::Insert(&in_freelist->free_nodes, in_node, insert_index);

        // merge backwards
        for (uint32_t i = start_index; i != -1; i--)
        {
            Node *curr = &in_freelist->free_nodes.data[i];

            if ((curr->start + curr->size) == start)
            {
                start = curr->start;
                start_index = i;
            }
        }
        uint32_t size = in_node.size;
        uint32_t count = 0;

        // merge forward
        {
            for (size_t end = start_index; end < in_freelist->free_nodes.size - 1; end++)
            {
                Node *curr = &in_freelist->free_nodes.data[end];
                Node *next = &in_freelist->free_nodes.data[end + 1];

                if ((curr->start + curr->size) != next->start)
                    break;

                size += curr->size;
                count++;
            }
        }

        const uint32_t remove_range = count;
        const uint32_t remove_index = start_index + 1;

        if (remove_index < in_freelist->free_nodes.size)
        {
            DArray<Node>::RemoveRange(&in_freelist->free_nodes, remove_index, remove_range);
        }

        in_freelist->free_nodes.data[start_index].start = start;
        in_freelist->free_nodes.data[start_index].size = size;
    }

    static bool AllocBlock(FreeList *in_freelist, const size_t size, Node *out_node)
    {
        if (in_freelist->free_nodes.size == 0)
        {
            return false;
        }

        Node *node = nullptr;
        size_t node_idx = -1;

        for (size_t i = 0; i < in_freelist->free_nodes.size; ++i)
        {
            Node *curr = &in_freelist->free_nodes.data[i];

            if (curr->size < size)
                continue;

            node = curr;
            node_idx = i;
            break;
        }

        if (node == nullptr)
        {
            return false;
        }

        in_freelist->used_mem += size;

        // add use node
        {
            Node use = {};
            use.size = (uint32_t)size;
            use.start = node->start;

            size_t used_count = in_freelist->used_nodes.size;

            if (in_freelist->used_nodes.data[used_count - 1].start < use.start)
            {
                DArray<Node>::Add(&in_freelist->used_nodes, use);
            }
            else
            {
                size_t insert_index = -1;
                FindInsertIndex(&in_freelist->used_nodes, use, &insert_index);

                DArray<Node>::Insert(&in_freelist->used_nodes, use, insert_index);
            }

            *out_node = use;
        }

        // edit free node
        if (size == node->size)
        {
            DArray<Node>::RemoveAt(&in_freelist->free_nodes, node_idx);
        }
        else
        {
            node->start += (uint32_t)size;
            node->size -= (uint32_t)size;
        }

        return true;
    }

    static void Destroy(FreeList *out_freelist)
    {
        DArray<Node>::Destroy(&out_freelist->free_nodes);
        DArray<Node>::Destroy(&out_freelist->used_nodes);
        *out_freelist = {};
    }

private:
    // Returns the index where we should insert the node
    static void FindInsertIndex(DArray<Node> *in_nodes, Node in_node, size_t *out_index)
    {
        if (in_nodes->size == 0)
        {
            *out_index = 0;
            return;
        }

        size_t best_index = 0;

        size_t low = 0;
        size_t high = in_nodes->size - 1;

        size_t node_end = (in_node.start + in_node.size);

        while (low <= high)
        {
            size_t index = low + ((high - low) / 2);

            Node *curr = &in_nodes->data[index];

            if (curr->start == node_end)
            {
                best_index = index;
                break;
            }

            if ((curr->start + curr->size) == in_node.start)
            {
                best_index = index + 1;
                break;
            }

            if (in_node.start < curr->start)
            {
                best_index = index;
                high = index - 1;
            }
            else
            {
                best_index = index + 1;
                low = index + 1;
            }
        }

        *out_index = best_index;
    }

    static void FindRemoveIndex(DArray<Node> *in_nodes, Node in_node, size_t *out_index)
    {
        if (in_nodes->size == 0)
        {
            *out_index = -1;
            return;
        }

        size_t low = 0;
        size_t high = in_nodes->size - 1;

        while (low <= high)
        {
            size_t index = low + ((high - low) / 2);

            Node *curr = &in_nodes->data[index];

            if (CoreContext::mem_compare(curr, &in_node, sizeof(Node)))
            {
                *out_index = index;
                return;
            }

            if (in_node.start < curr->start)
            {
                high = index - 1;
            }
            else
            {
                low = index + 1;
            }
        }

        *out_index = -1;
    }
};