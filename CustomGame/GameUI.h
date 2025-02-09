#pragma once
#include <Core/UI/UILayout.h>
#include <Core/UI/FlowLayout.h>
#include "EntryPoint.h"

struct GameUI
{
    static void Build(EntryPoint* entry)
    {
        // screen node
        LayoutNode screen_node = {};
        {
            screen_node.resolved_rect.x = 0;
            screen_node.resolved_rect.y = 0;
            screen_node.resolved_rect.width = Global::platform.window.width;
            screen_node.resolved_rect.height = Global::platform.window.height;
            screen_node.is_h_resolved = true;
            screen_node.is_w_resolved = true;
            DArray<LayoutNode>::Create(1 , &screen_node.sub_nodes , Global::alloc_toolbox.frame_allocator);
        }

        // root node
        LayoutNode root_node = {};
        {
            root_node.option.x = { PointOrigin::Absolute, ValueUnit::Pixels, 0 };
            root_node.option.y = { PointOrigin::Absolute, ValueUnit::Pixels, 0 };
            root_node.option.width = { SizeRule::Parent, ValueUnit::Percentage, 1 };
            root_node.option.height = { SizeRule::Value, ValueUnit::Pixels, 300 };
            DArray<LayoutNode>::Create(1 , &root_node.sub_nodes , Global::alloc_toolbox.frame_allocator);
        }

        // child nodes
        for(size_t i = 0; i < 4 ; ++i)
        {
            LayoutNode test_node = {};
            {
                float width = 64.0 * (i + 1);
                test_node.option.x = { PointOrigin::Layout, ValueUnit::Pixels, 32 };
                test_node.option.y = { PointOrigin::Layout, ValueUnit::Pixels, 32 };
                test_node.option.width = { SizeRule::Value, ValueUnit::Pixels, width};
                test_node.option.height = { SizeRule::Value, ValueUnit::Pixels, 64 };

                test_node.parent = &entry->ui_root.root;
                DArray<LayoutNode>::Add(&root_node.sub_nodes , test_node);
            }
        }

        // auto width node
        {
            LayoutNode test_node = {};
            {
                test_node.option.x = { PointOrigin::Layout, ValueUnit::Pixels, 32 };
                test_node.option.y = { PointOrigin::Layout, ValueUnit::Pixels, 32 };
                test_node.option.width = { SizeRule::Layout, ValueUnit::Pixels, 0};
                test_node.option.height = { SizeRule::Value, ValueUnit::Pixels, 64 };

                test_node.parent = &entry->ui_root.root;
                DArray<LayoutNode>::Add(&root_node.sub_nodes , test_node);
            }
        }

        entry->ui_root.root = root_node;
        entry->ui_root.mesh = &entry->plane_mesh;
        entry->ui_root.texture = &entry->ui_texture;
        entry->ui_root.shader_builder = &entry->ui_shader_builder;

        LayoutState ui_state = {};
        ui_state.screen_node = screen_node;
        ui_state.root = &entry->ui_root.root;
        ui_state.root->parent = &ui_state.screen_node;
        DArray<LayoutNode>::Add(&ui_state.screen_node.sub_nodes , *ui_state.root);

        FlowLayout::Flow(&entry->ui_root.root , &ui_state);
    };
};