#pragma once
#include "CustomGameState.h"
#include <Core/UI/UILayout.h>

struct GameUI
{
    static void Build(CustomGameState* game_state)
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
        root_node.option.x =
        {
            LayoutOrigin::Screen,
            LayoutSize::Value,
            LayoutUnit::Pixels,
            0
        };

        root_node.option.y =
        {
            LayoutOrigin::Screen,
            LayoutSize::Value,
            LayoutUnit::Pixels,
            0
        };

        root_node.option.width = 
        {
            LayoutOrigin::Screen,
            LayoutSize::Value,
            LayoutUnit::Percentage,
            1
        };

        root_node.option.height = 
        {
            LayoutOrigin::Screen,
            LayoutSize::Value,
            LayoutUnit::Pixels,
            300
        };

        game_state->ui_root = root_node;
        
        LayoutState ui_state = {};
        ui_state.screen_node = screen_node;
        ui_state.root = &game_state->ui_root;

        LayoutBuilder::ComputeLayout(&game_state->ui_root , &ui_state);
    };
};