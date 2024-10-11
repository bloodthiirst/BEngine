#pragma once
#include <Containers/DArray.h>
#include <Maths/Rect.h>
#include <Maths/Vector2.h>
#include <Maths/Matrix4x4.h>
#include <Core/Renderer/Context/RendererContext.h>
#include <Core/Renderer/Context/VulkanContext.h>

enum class LayoutDirection
{
    Horizontal,
    Vertical
};

enum class LayoutRule
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
    Absolute,
    Parent,
    Root,
    Screen
};

struct LayoutValue
{
    LayoutOrigin origin;
    LayoutRule layout_rule;
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

struct Mesh3D;
struct ShaderBuilder;
struct Texture;

struct RootLayoutNode
{
    LayoutNode root;
    Mesh3D* mesh;
    ShaderBuilder* shader_builder;
    Texture* texture;
    FreeList::Node instances_data;

    void GetAllRectRecursive(LayoutNode* root_node , DArray<Matrix4x4>* inout_mats)
    {
        Rect rect = root_node->resolved_rect;

        Matrix4x4 ui_rect_mat = Matrix4x4(
            {rect.size.x, 0, 0, rect.pos.x},
            {0, rect.size.y, 0, rect.pos.y},
            {0, 0, 1, 0},
            {0, 0, 0, 1});

        DArray<Matrix4x4>::Add(inout_mats , ui_rect_mat);

        for(size_t i = 0; i < root_node->sub_nodes.size ; ++i)
        {
            GetAllRectRecursive(&root_node->sub_nodes.data[i] , inout_mats);
        }
    }

    DrawMesh GetDraw()
    {
        VulkanContext* ctx = (VulkanContext*)Global::backend_renderer.user_data;
        DArray<Matrix4x4> rects_to_render = {};
        DArray<Matrix4x4>::Create(100 , &rects_to_render , Global::alloc_toolbox.frame_allocator);

        GetAllRectRecursive(&root , &rects_to_render);

        const size_t size_for_rects = sizeof(Matrix4x4) * rects_to_render.size;

        DrawMesh draw = {};
        draw.mesh = mesh;
        draw.shader_builder = shader_builder;
        draw.texture = texture;
        draw.instances_data = instances_data;
        draw.instances_count = rects_to_render.size;

        assert(draw.instances_count == 2);

        VkCommandPool pool = ctx->physical_device_info.command_pools_info.graphicsCommandPool;
        VkQueue queue = ctx->physical_device_info.queues_info.graphics_queue;
        Buffer::Load(instances_data.start, size_for_rects , rects_to_render.data, 0, &ctx->staging_buffer); 
        Buffer::Copy(pool , {} , queue , &ctx->staging_buffer , 0 , &ctx->descriptors_buffer , instances_data.start , size_for_rects);
        return draw;
    }
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
                float val = {};

                switch(in_node->option.width.unit)
                {   
                    case LayoutUnit::Percentage: 
                    {
                        switch(in_node->option.width.origin)
                        {
                            case LayoutOrigin::Parent : val = (in_node->parent->resolved_rect.width * in_node->option.width.value); break;
                            case LayoutOrigin::Absolute : val = in_node->option.width.value; break;
                            case LayoutOrigin::Root : val = (in_state->root->resolved_rect.width * in_node->option.width.value); break;
                            case LayoutOrigin::Screen : val = (in_state->screen_node.resolved_rect.width * in_node->option.width.value); break;                 
                        }
                        break;
                    }
                    case LayoutUnit::Pixels: 
                    {
                        switch(in_node->option.width.origin)
                        {
                            case LayoutOrigin::Parent : val = (in_node->parent->resolved_rect.width + in_node->option.width.value); break;
                            case LayoutOrigin::Absolute : val = in_node->option.width.value; break;
                            case LayoutOrigin::Root : val = (in_state->root->resolved_rect.width + in_node->option.width.value); break;
                            case LayoutOrigin::Screen : val = (in_state->screen_node.resolved_rect.width + in_node->option.width.value); break;                 
                        }
                        break;
                    }
                }

                rect.width = val;
            }

            // pos x
            {
                float val = {};

                switch(in_node->option.x.unit)
                {   
                    case LayoutUnit::Percentage: 
                    {
                        switch(in_node->option.x.origin)
                        {
                            case LayoutOrigin::Parent : val = (in_node->parent->resolved_rect.width * in_node->option.x.value); break;
                            case LayoutOrigin::Absolute : val = in_node->option.x.value; break;
                            case LayoutOrigin::Root : val = (in_state->root->resolved_rect.width * in_node->option.x.value); break;
                            case LayoutOrigin::Screen : val = (in_state->screen_node.resolved_rect.width * in_node->option.x.value); break;                 
                        }
                        break;
                    }
                    case LayoutUnit::Pixels: 
                    {
                        switch(in_node->option.x.origin)
                        {
                            case LayoutOrigin::Parent : val = (in_node->parent->resolved_rect.x + in_node->option.x.value); break;
                            case LayoutOrigin::Absolute : val = in_node->option.x.value; break;
                            case LayoutOrigin::Root : val = (in_state->root->resolved_rect.x + in_node->option.x.value); break;
                            case LayoutOrigin::Screen : val = (in_state->screen_node.resolved_rect.x + in_node->option.x.value); break;                 
                        }
                        break;
                    }
                }

                rect.x = val;
            }
        }

        // if height isn't computed yet
        if (!in_node->is_h_resolved)
        {          
            // height
            {
                float val = {};

                switch(in_node->option.height.unit)
                {   
                    case LayoutUnit::Percentage: 
                    {
                        switch(in_node->option.height.origin)
                        {
                            case LayoutOrigin::Parent : val = (in_node->parent->resolved_rect.height * in_node->option.height.value); break;
                            case LayoutOrigin::Absolute : val = in_node->option.height.value; break;
                            case LayoutOrigin::Root : val = (in_state->root->resolved_rect.height * in_node->option.height.value); break;
                            case LayoutOrigin::Screen : val = (in_state->screen_node.resolved_rect.height * in_node->option.height.value); break;                 
                        }
                        break;
                    }
                    case LayoutUnit::Pixels: 
                    {
                        switch(in_node->option.height.origin)
                        {
                            case LayoutOrigin::Parent : val = (in_node->parent->resolved_rect.height + in_node->option.height.value); break;
                            case LayoutOrigin::Absolute : val = in_node->option.height.value; break;
                            case LayoutOrigin::Root : val = (in_state->root->resolved_rect.height + in_node->option.height.value); break;
                            case LayoutOrigin::Screen : val = (in_state->screen_node.resolved_rect.height + in_node->option.height.value); break;                 
                        }
                        break;
                    }
                }

                rect.height = val;
            }

            // pos y
            {
                float val = {};

                switch(in_node->option.y.unit)
                {   
                    case LayoutUnit::Percentage: 
                    {
                        switch(in_node->option.y.origin)
                        {
                            case LayoutOrigin::Parent : val = (in_node->parent->resolved_rect.height * in_node->option.y.value); break;
                            case LayoutOrigin::Absolute : val = in_node->option.y.value; break;
                            case LayoutOrigin::Root : val = (in_state->root->resolved_rect.height * in_node->option.y.value); break;
                            case LayoutOrigin::Screen : val = (in_state->screen_node.resolved_rect.height * in_node->option.y.value); break;                 
                        }
                        break;
                    }
                    case LayoutUnit::Pixels: 
                    {
                        switch(in_node->option.y.origin)
                        {
                            case LayoutOrigin::Parent : val = (in_node->parent->resolved_rect.y + in_node->option.y.value); break;
                            case LayoutOrigin::Absolute : val = in_node->option.y.value; break;
                            case LayoutOrigin::Root : val = (in_state->root->resolved_rect.y + in_node->option.y.value); break;
                            case LayoutOrigin::Screen : val = (in_state->screen_node.resolved_rect.y + in_node->option.y.value); break;                 
                        }
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