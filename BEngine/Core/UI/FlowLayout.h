#pragma once
#include <Defer/Defer.h>
#include "UILayout.h"

struct FlowLayout
{
    static void Flow(LayoutNode *in_node, LayoutState *in_state)
    {
        FlowLayout::FlowWidth(in_node, in_state);
        FlowLayout::FlowHeight(in_node, in_state);
        FlowLayout::FlowX(in_node, in_state);
        FlowLayout::FlowY(in_node, in_state);

        for (size_t i = 0; i < in_node->sub_nodes.size; ++i)
        {
            LayoutNode *curr = &in_node->sub_nodes.data[i];
            Flow(curr, in_state);
        }
    }

    static void FlowWidth(LayoutNode *in_node, LayoutState *in_state)
    {
        // maybe resolve width first ?
        Rect rect = in_node->resolved_rect;

        // skip if already computed
        if (in_node->is_w_resolved)
        {
            return;
        }

        float val = {};

        switch (in_node->option.width.rule)
        {
        case SizeRule::Value:
        {
            assert(in_node->option.width.unit != ValueUnit::Percentage);

            switch (in_node->option.width.unit)
            {
            case ValueUnit::Pixels:
            {
                val = in_node->option.width.value;
                goto end;
                break;
            }
            }
            break;
        }

        case SizeRule::Parent:
        {
            switch (in_node->option.width.unit)
            {
            case ValueUnit::Percentage:
            {
                val = in_node->parent->resolved_rect.width * in_node->option.width.value;
                goto end;
                break;
            }
            case ValueUnit::Pixels:
            {
                val = in_node->parent->resolved_rect.width + in_node->option.width.value;
                goto end;
                break;
            }
            default:
                break;
            }
            break;
        }

        case SizeRule::Content:
        {
            float sum = 0;
            for (size_t i = 0; i < in_node->sub_nodes.size; ++i)
            {
                LayoutNode *curr = &in_node->sub_nodes.data[i];
                FlowWidth(curr, in_state);
                assert(curr->is_w_resolved);
                sum += curr->resolved_rect.width;
            }

            val = sum;
            in_node->resolved_rect.width = val;
            in_node->is_w_resolved = true;
            return;
        }
        }

    end:
        in_node->resolved_rect.width = val;
        in_node->is_w_resolved = true;

        ArenaCheckpoint check = Global::alloc_toolbox.GetArenaCheckpoint();
        DEFER([&]()
              { Global::alloc_toolbox.ResetArenaOffset(&check); });

        DArray<LayoutNode *> auto_w = {};
        DArray<LayoutNode *>::Create(in_node->sub_nodes.size, &auto_w, Global::alloc_toolbox.frame_allocator);

        float left_w = in_node->resolved_rect.width;

        for (size_t i = 0; i < in_node->sub_nodes.size; ++i)
        {
            LayoutNode *curr = &in_node->sub_nodes.data[i];

            if (curr->option.width.rule == SizeRule::Layout)
            {
                DArray<LayoutNode *>::Add(&auto_w, curr);
                continue;
            }

            FlowWidth(curr, in_state);

            assert(curr->is_w_resolved);
            left_w -= curr->resolved_rect.width;
        }

        float w_per_auto = left_w / auto_w.size;

        for (size_t i = 0; i < auto_w.size; ++i)
        {
            LayoutNode *curr = auto_w.data[i];
            curr->resolved_rect.width = w_per_auto;
            curr->is_w_resolved = true;
        }
    }

    static void FlowHeight(LayoutNode *in_node, LayoutState *in_state)
    {
        // maybe resolve width first ?
        Rect rect = in_node->resolved_rect;

        // skip if already computed
        if (in_node->is_h_resolved)
        {
            return;
        }

        float val = {};

        switch (in_node->option.height.rule)
        {
        case SizeRule::Value:
        {
            assert(in_node->option.height.unit != ValueUnit::Percentage);

            switch (in_node->option.height.unit)
            {
            case ValueUnit::Pixels:
            {
                val = in_node->option.height.value;
                goto end;
                break;
            }
            }
            break;
        }

        case SizeRule::Parent:
        {
            switch (in_node->option.height.unit)
            {
            case ValueUnit::Percentage:
            {
                val = in_node->parent->resolved_rect.height * in_node->option.height.value;
                goto end;
                break;
            }
            case ValueUnit::Pixels:
            {
                val = in_node->parent->resolved_rect.height + in_node->option.height.value;
                goto end;
                break;
            }
            default:
                break;
            }
            break;
        }

        case SizeRule::Content:
        {
            float sum = 0;
            for (size_t i = 0; i < in_node->sub_nodes.size; ++i)
            {
                LayoutNode *curr = &in_node->sub_nodes.data[i];
                FlowHeight(curr, in_state);
                assert(curr->is_h_resolved);
                sum += curr->resolved_rect.height;
            }

            val = sum;
            in_node->resolved_rect.height = val;
            in_node->is_w_resolved = true;
            return;
        }
        }

    end:
        in_node->resolved_rect.height = val;
        in_node->is_h_resolved = true;

        ArenaCheckpoint check = Global::alloc_toolbox.GetArenaCheckpoint();
        DEFER([&]()
              { Global::alloc_toolbox.ResetArenaOffset(&check); });

        DArray<LayoutNode *> auto_h = {};
        DArray<LayoutNode *>::Create(in_node->sub_nodes.size, &auto_h, Global::alloc_toolbox.frame_allocator);

        float left_h = in_node->resolved_rect.height;

        for (size_t i = 0; i < in_node->sub_nodes.size; ++i)
        {
            LayoutNode *curr = &in_node->sub_nodes.data[i];

            if (curr->option.height.rule == SizeRule::Layout)
            {
                DArray<LayoutNode *>::Add(&auto_h, curr);
                continue;
            }

            FlowHeight(curr, in_state);

            assert(curr->is_h_resolved);
            left_h -= curr->resolved_rect.height;
        }

        float h_per_auto = left_h / auto_h.size;

        for (size_t i = 0; i < auto_h.size; ++i)
        {
            LayoutNode *curr = auto_h.data[i];
            curr->resolved_rect.height = h_per_auto;
            curr->is_h_resolved = true;
        }
    }

    static void FlowX(LayoutNode *in_node, LayoutState *in_state)
    {
        assert(in_node->is_h_resolved && in_node->is_w_resolved);

        LayoutNode *origin = {};
        switch (in_node->option.x.origin)
        {
        case PointOrigin::Layout:
        {
            goto end;
            break;
        }
        case PointOrigin::Absolute:
        {
            origin = in_node->parent;
            break;
        }
        case PointOrigin::Root:
        {
            origin = in_state->root;
            break;
        }
        case PointOrigin::Screen:
        {
            origin = &in_state->screen_node;
            break;
        }
        }

        switch (in_node->option.x.unit)
        {
        case ValueUnit::Pixels:
        {
            in_node->resolved_rect.x = origin->resolved_rect.x + in_node->option.x.value;
            goto end;
            break;
        }
        case ValueUnit::Percentage:
        {
            in_node->resolved_rect.x = origin->resolved_rect.x + (origin->resolved_rect.width * in_node->option.x.value);
            goto end;
            break;
        }
        break;
        }
    end:

        float x_acc = in_node->resolved_rect.x;

        for (size_t i = 0; i < in_node->sub_nodes.size; ++i)
        {
            LayoutNode *curr = &in_node->sub_nodes.data[i];
            curr->resolved_rect.x = x_acc;

            if(curr->option.x.origin == PointOrigin::Layout && in_node->option.direction == LayoutDirection::Horizontal)
            {
                x_acc += curr->resolved_rect.width;
            }        

            FlowX(curr , in_state);
        }
    }

    static void FlowY(LayoutNode *in_node, LayoutState *in_state)
    {
        assert(in_node->is_h_resolved && in_node->is_w_resolved);

        LayoutNode *origin = {};
        switch (in_node->option.y.origin)
        {
        case PointOrigin::Layout:
        {
            goto end;
            break;
        }
        case PointOrigin::Absolute:
        {
            origin = in_node->parent;
            break;
        }
        case PointOrigin::Root:
        {
            origin = in_state->root;
            break;
        }
        case PointOrigin::Screen:
        {
            origin = &in_state->screen_node;
            break;
        }
        }

        switch (in_node->option.y.unit)
        {
        case ValueUnit::Pixels:
        {
            in_node->resolved_rect.y = origin->resolved_rect.y + in_node->option.y.value;
            goto end;
            break;
        }
        case ValueUnit::Percentage:
        {
            in_node->resolved_rect.y = origin->resolved_rect.y + (origin->resolved_rect.height * in_node->option.y.value);
            goto end;
            break;
        }
        break;
        }
    end:

        float y_acc = in_node->resolved_rect.y;

        for (size_t i = 0; i < in_node->sub_nodes.size; ++i)
        {
            LayoutNode *curr = &in_node->sub_nodes.data[i];
            curr->resolved_rect.y = y_acc;
            
            if(curr->option.y.origin == PointOrigin::Layout && in_node->option.direction == LayoutDirection::Vertical)
            {
                y_acc += curr->resolved_rect.height;
            }

            FlowY(curr , in_state);
        }
    }

};