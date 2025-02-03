#pragma once
#include "RenderGraphBuilder.h"
#include "../Subpasses/UISubpass.h"
#include "../Renderpass/BasicRenderpass.h"
#include "../../Global/Global.h"

struct BasicRenderGraph
{
    static inline char *renderpass_id = "UI Renderpass";
    static inline char *color_attachment_id = "Color attachment ID";
    static inline char *depth_attachment_id = "Depth attachment ID";

    static RenderGraphBuilder Create()
    {
        VulkanContext *ctx = (VulkanContext *)Global::backend_renderer.user_data;

        VkAttachmentDescription color_attachment_desc = {};
        VkAttachmentReference color_reference = {};

        // color attachement
        {
            color_attachment_desc.format = ctx->swapchain_info.surfaceFormat.format;
            color_attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
            // loadOp defines what to do to the texture when we load it
            // we can clear OnLoad , keep the previous content , or don't care
            // color
            color_attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

            // storeOp defines what to do to the texture when we store it
            // we can "store it" , (which seems redundant in terms of naming) but means keep the content of this texture to be used by things to come after
            // or don't care , which means that the content wont be used after we're done using it
            color_attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

            // we're not usin stencil for now , so don't care
            // stencil
            color_attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            color_attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            color_attachment_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;     // since this is a brand new "create" , we're not creating this renderpass using an existing image from a previous renderpass , so no "expected" image layout
            color_attachment_desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // what we're gonna transition to after the renderpass , it's a layout format in memory
            color_attachment_desc.flags = 0;

            // specifies the :
            color_reference.attachment = 0;                                    // attachement index
            color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // layout
        }

        VkAttachmentDescription depth_attachment_desc = {};
        VkAttachmentReference depth_reference = {};

        // depth attachement
        {
            depth_attachment_desc.format = ctx->physical_device_info.depthFormat;
            depth_attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;

            // loadOp define what to do to the texture when we load it
            // we can clear OnLoad , keep the previous content , or don't care
            // color
            depth_attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

            // storeOp definew what to do to the textre when we store it
            // we can "store i" , which seems redundant but means keep the content of this texture to be used by things to come after
            // or don't care , which means that the content wont be used after we're done using it
            depth_attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

            // we're not usin stencil for now , so don't care
            // stencil
            depth_attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depth_attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            depth_attachment_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                      // since this is a brand new "create" , we're not creating this renderpass using an existing image from a previous renderpass , so no "expected" image layout
            depth_attachment_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // what we're gonna transition to after the renderpass , it's a layout format in memory
            depth_attachment_desc.flags = 0;

            // specifies the
            depth_reference.attachment = 1;                                            // attachement index
            depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // layout
        }

        Vector2Int screen = {};
        screen.x = Global::platform.window.width;
        screen.y = Global::platform.window.height;

        Subpass ui_pass = {};
        bool created_pass = UISubpass::Create(ctx, &ui_pass);
        assert(created_pass);

        BasicRenderpassParams* params = Global::alloc_toolbox.HeapAlloc<BasicRenderpassParams>();
        {
            params->sync_window_size = true;
            params->clearColor = Color{0.2f, 0.2f, 0.2f, 1};
            params->area = Rect{0, 0, (float)screen.x, (float)screen.y};
            params->stencil = 0;
            params->depth = 1;
            params->color_id = color_attachment_id;
            params->depth_id = depth_attachment_id;
        }

        RenderGraphBuilder graph_builder = RenderGraphBuilder::Create(Global::alloc_toolbox.frame_allocator);

        graph_builder
            .AddRenderpass(renderpass_id, params , BasicRenderpass::Builder)
            ->AddRenderTarget(color_attachment_id, 0, color_attachment_desc)
            ->AddRenderTarget(depth_attachment_id, 1, depth_attachment_desc)
            ->AddSubpass(ui_pass)
            ->Done();

        return graph_builder;
    }
};