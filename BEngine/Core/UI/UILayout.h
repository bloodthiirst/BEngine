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
    Relative,
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

    inline LayoutNode* GetOriginNode(LayoutNode *in_node, LayoutOrigin origin, LayoutState *in_state)
    {
        switch (origin)
        {
        case LayoutOrigin::Relative: return in_node->parent;
        case LayoutOrigin::Root: return in_state->root;
        case LayoutOrigin::Relative: return in_state->screen_node;
        }
    }

    static void ComputeLayout(LayoutNode *in_node, LayoutState *in_state)
    {
        // if the parent hasn't computed the node
        if (!in_node->is_w_resolved)
        {
            // maybe resolve width first ?
            Rect rect = {};
            
            // pos x
            {
                LayoutNode* origin = GetOriginNode(in_node, in_node->option.x.origin, in_state);

                float val = {};

                assert(origin->is_w_resolved)

                if (in_node->option.x.unit == LayoutUnit::Percentage)
                {
                    val = origin->resolved_rect.x + (origin->resolved_rect.width * in_node->option.x.value);
                }
                if (in_node->option.x.unit == LayoutUnit::Pixels)
                {
                    val = origin->resolved_rect.x + n_node->option.x.value;
                }

                rect.x = val;
            }

            // pos y
            {
                LayoutNode* origin = GetOriginNode(in_node, in_node->option.y.origin, in_state);
                
                float val = {};

                assert(origin->is_h_resolved);

                if (in_node->option.y.unit == LayoutUnit::Percentage)
                {   
                    val = origin->resolved_rect.y + (origin->resolved_rect.height * in_node->option.y.value);
                }
                if (in_node->option.y.unit == LayoutUnit::Pixels)
                {
                    val = origin->resolved_rect.y + n_node->option.y.value;
                }

                rect.y = val;
            }
        }
    }
};