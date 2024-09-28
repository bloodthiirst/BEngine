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
        
        const Vector2 pos = game_state->ui_root.resolved_rect.pos;
        const Vector2 size = game_state->ui_root.resolved_rect.size;

        const Vector2 tr = pos + size;
        const Vector2 tl = pos + Vector2(0 , size.y);
        const Vector2 br = pos + Vector2(size.x , 0);
        const Vector2 bl = pos;

        Vertex3D verts[] = 
        {
            {tr , Vector2(1.0f, 1.0f)}, // TOP RIGHT
            {tl , Vector2(0.0f, 1.0f)}, // TOP LEFT
            {br , Vector2(1.0f, 0.0f)}, // BOT RIGHT
            {bl , Vector2(0.0f, 0.0f)}  // BOT LEFT
        }; 

        uint32_t indicies[] = {
            2,
            1,
            0,

            2,
            3,
            1,
        };

        ArrayView<Vertex3D> vert_view = {};
        vert_view.data = verts;
        vert_view.size = 4;

        ArrayView<uint32_t> ind_view = {};
        ind_view.data = indicies;
        ind_view.size = 6;
        
        Mesh3D::FreeData(&game_state->plane_mesh);
        Mesh3D::AllocData(&game_state->plane_mesh , vert_view , ind_view);
    };
};