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

enum class ValueUnit
{
    Percentage,
    Pixels
};

enum class PointOrigin
{
    Absolute,
    Layout,
    Root,
    Screen
};

enum class SizeRule
{
    Parent,
    Content,
    Layout,
    Value
};

struct LayoutPoint
{
    PointOrigin origin;
    ValueUnit unit;
    float value;
};

struct LayoutSize
{
    SizeRule rule;
    ValueUnit unit;
    float value;
};

struct LayoutOption
{
    LayoutPoint x;
    LayoutPoint y;
    LayoutSize width;
    LayoutSize height;
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

        //assert(draw.instances_count == 3);

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
    static LayoutNode* GetOriginNode(LayoutNode *in_node, PointOrigin origin, LayoutState *in_state)
    {
        switch (origin)
        {
        case PointOrigin::Layout: return in_node->parent;
        case PointOrigin::Root: return in_state->root;
        case PointOrigin::Screen: return &in_state->screen_node;
        }

        assert(false);

        return nullptr;
    }

};