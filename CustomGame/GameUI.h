#pragma once
#include <Core/UI/UILayout.h>
#include "EntryPoint.h"

struct GameUI
{
    static void Build(EntryPoint* entry)
    {
        // screen node
        LayoutNode screen_node = {};
        screen_node.resolved_rect.x = 0;
        screen_node.resolved_rect.y = 0;
        screen_node.resolved_rect.width = Global::platform.window.width;
        screen_node.resolved_rect.height = Global::platform.window.height;
        screen_node.is_h_resolved = true;
        screen_node.is_w_resolved = true;
        
        // root node
        LayoutNode root_node = {};
        root_node.option.x = { LayoutOrigin::Screen , LayoutRule::Value,LayoutUnit::Pixels, 0 };
        root_node.option.y = { LayoutOrigin::Screen , LayoutRule::Value,LayoutUnit::Pixels, 0 };
        root_node.option.width = { LayoutOrigin::Screen , LayoutRule::Value,LayoutUnit::Percentage, 1 };
        root_node.option.height = { LayoutOrigin::Absolute , LayoutRule::Value,LayoutUnit::Pixels, 300 };
        DArray<LayoutNode>::Create(1 , &root_node.sub_nodes , Global::alloc_toolbox.frame_allocator);

        // test node
        LayoutNode test_node = {};
        test_node.option.x = { LayoutOrigin::Parent , LayoutRule::Value, LayoutUnit::Pixels, 32 };
        test_node.option.y = { LayoutOrigin::Parent , LayoutRule::Value, LayoutUnit::Pixels, 32 };
        test_node.option.width = { LayoutOrigin::Parent , LayoutRule::Value, LayoutUnit::Pixels, -64};
        test_node.option.height = { LayoutOrigin::Parent , LayoutRule::Value, LayoutUnit::Pixels, -64 };

        test_node.parent = &entry->ui_root.root;
        DArray<LayoutNode>::Add(&root_node.sub_nodes , test_node);

        entry->ui_root.root = root_node;
        entry->ui_root.mesh = &entry->plane_mesh;
        entry->ui_root.texture = &entry->ui_texture;
        entry->ui_root.shader_builder = &entry->ui_shader_builder;

        LayoutState ui_state = {};
        ui_state.screen_node = screen_node;
        ui_state.root = &entry->ui_root.root;

        LayoutBuilder::ComputeLayout(&entry->ui_root.root , &ui_state);
    };
};