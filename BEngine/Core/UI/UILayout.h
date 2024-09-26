#pragma once
#include <Containers/DArray.h>
#include <Maths/Rect.h>

enum class LayoutUnit
{
    Percentage,
    Pixels,
    Content,
    Auto
};

enum class LayoutOrigin
{
    Relative,
    Screen
};

struct LayoutValue
{
    LayoutOrigin origin;
    LayoutUnit unit;
    float value;
};

struct LayoutOption
{
    LayoutValue x;
    LayoutValue y;
    LayoutValue width;
    LayoutValue height;
};

struct LayoutNode
{
    LayoutOption option;
    Rect resolved_rect;
    bool is_dirty;
    DArray<LayoutNode> sub_nodes;
};

struct LayoutState
{
    LayoutNode* root;
    Allocator node_allocator;
};

struct LayoutBuilder
{
    static void ComputeLayout(LayoutNode* inout_node , LayoutState* in_state)
    {
    }
};