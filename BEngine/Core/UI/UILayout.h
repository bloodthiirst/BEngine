#pragma once
#include <Containers/DArray.h>
#include <Maths/Rect.h>
#include <Maths/Vector2.h>

enum class LayoutDirection
{
    Horizontal,
    Vertical
};

enum class LayoutSize
{
    Parent,
    Content,
    Value
};

enum class LayoutUnit
{
    Percentage,
    Pixels,
};

enum class LayoutOrigin
{
    Parent,
    Root,
    Screen
};

struct LayoutValue
{
    LayoutOrigin origin;
    LayoutSize size;
    LayoutUnit unit;
    float value;
};

struct LayoutOption
{
    LayoutValue x;
    LayoutValue y;
    LayoutValue width;
    LayoutValue height;
    LayoutDirection direction;
};

struct LayoutNode
{
    LayoutNode *parent;
    LayoutOption option;
    DArray<LayoutNode> sub_nodes;
    Rect resolved_rect;
    bool is_dirty;
    bool is_w_resolved;
    bool is_h_resolved;
};

struct LayoutState
{
    LayoutNode *root;
    LayoutNode screen_node;
    Allocator node_allocator;
};

struct LayoutBuilder
{
    static LayoutNode* GetOriginNode(LayoutNode *in_node, LayoutOrigin origin, LayoutState *in_state)
    {
        switch (origin)
        {
        case LayoutOrigin::Parent: return in_node->parent;
        case LayoutOrigin::Root: return in_state->root;
        case LayoutOrigin::Screen: return &in_state->screen_node;
        }

        assert(false);

        return nullptr;
    }

    static void ComputeLayout(LayoutNode *in_node, LayoutState *in_state)
    {
        // maybe resolve width first ?
        Rect rect = in_node->resolved_rect;

        // if width isn't computed yet
        if (!in_node->is_w_resolved)
        {          
            // width
            {
                LayoutNode* origin = GetOriginNode(in_node, in_node->option.width.origin, in_state);
                
                float val = {};

                switch(in_node->option.width.unit)
                {   
                    case LayoutUnit::Percentage: 
                    {
                        val = (origin->resolved_rect.width * in_node->option.width.value);
                        break;
                    }
                    case LayoutUnit::Pixels: 
                    {
                        val = in_node->option.width.value;
                        break;
                    }
                }

                rect.width = val;
            }

            // pos x
            {
                LayoutNode* origin = GetOriginNode(in_node, in_node->option.x.origin, in_state);

                float val = {};

                switch(in_node->option.x.unit)
                {   
                    case LayoutUnit::Percentage: 
                    {
                        val = origin->resolved_rect.x + (origin->resolved_rect.width * in_node->option.x.value);
                        break;
                    }
                    case LayoutUnit::Pixels: 
                    {
                        val = origin->resolved_rect.x + in_node->option.x.value;
                        break;
                    }
                }

                rect.x = val;
            }
        }

        // if height isn't computed yet
        if (!in_node->is_h_resolved)
        {          
            // width
            {
                LayoutNode* origin = GetOriginNode(in_node, in_node->option.height.origin, in_state);
                
                float val = {};

                switch(in_node->option.height.unit)
                {   
                    case LayoutUnit::Percentage: 
                    {
                        val = (origin->resolved_rect.height * in_node->option.height.value);
                        break;
                    }
                    case LayoutUnit::Pixels: 
                    {
                        val = in_node->option.height.value;
                        break;
                    }
                }

                rect.height = val;
            }

            // pos y
            {
                LayoutNode* origin = GetOriginNode(in_node, in_node->option.y.origin, in_state);

                float val = {};

                switch(in_node->option.y.unit)
                {   
                    case LayoutUnit::Percentage: 
                    {
                        val = origin->resolved_rect.y + (origin->resolved_rect.height * in_node->option.y.value);
                        break;
                    }
                    case LayoutUnit::Pixels: 
                    {
                        val = origin->resolved_rect.y + in_node->option.y.value;
                        break;
                    }
                }

                rect.y = val;
            }
        }

        in_node->resolved_rect = rect;

        for(size_t i = 0; i < in_node->sub_nodes.size; ++i)
        {
            LayoutNode* curr = &in_node->sub_nodes.data[i];
            ComputeLayout(curr , in_state);
        }
    }
};