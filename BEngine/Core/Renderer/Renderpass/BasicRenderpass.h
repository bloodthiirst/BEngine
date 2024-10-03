#pragma once
#include <Containers/DArray.h>
#include <Containers/HMap.h>
#include "../../Global/Global.h"
#include "../Renderpass/Renderpass.h"
#include "../CommandBuffer/CommandBuffer.h"
#include "../Buffer/Buffer.h"
#include "../Context/VulkanContext.h"
#include "../RenderGraph/RenderGraphBuilder.h"

struct BasicRenderpassParams
{
    bool sync_window_size;
    Rect area;
    Color clearColor;
    float depth;
    uint32_t stencil;
    StringView color_id;
    StringView depth_id;    
};


struct BasicRenderpass
{
    static inline const char *BASIC_RENDERPASS_ID = "BasicRenderPass - ID";

    static Renderpass Builder(RenderGraphBuilder* builder, RenderpassNode node)
    {
        VulkanContext* ctx = (VulkanContext*) Global::backend_renderer.user_data;
        BasicRenderpassParams* casted = (BasicRenderpassParams* ) node.params;

        Renderpass result = {};
        Create(ctx , builder , node , &result);

        return result;
    }

    static bool Create(VulkanContext *ctx, RenderGraphBuilder* builder, RenderpassNode node, Renderpass *out_renderpass)
    {
        *out_renderpass = {};

        out_renderpass->id = StringView::Create(BASIC_RENDERPASS_ID);

        BasicRenderpassParams* params = (BasicRenderpassParams*) node.params;
        
        // main subpass
        VkSubpassDescription subpassDesc = {};
        subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        // todo : we could have other attachement (input , resolve , preserve)

        VkAttachmentReference colorReference = {};
        RenderTargetInfo* color_lookup = {};
        {
            bool found = HMap<StringView,RenderTargetInfo>::TryGet(&node.render_targets , params->color_id , &color_lookup);
            assert(found);
            colorReference.attachment = color_lookup->index;
            colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }

        VkAttachmentReference depthReference = {};
        RenderTargetInfo* depth_lookup = {};
        {
            bool found = HMap<StringView,RenderTargetInfo>::TryGet(&node.render_targets , params->depth_id , &depth_lookup);
            assert(found);
            depthReference.attachment = depth_lookup->index;
            depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        subpassDesc.colorAttachmentCount = 1;
        subpassDesc.pColorAttachments = &colorReference;
        subpassDesc.pDepthStencilAttachment = &depthReference;

        // input from a shader
        subpassDesc.inputAttachmentCount = 0;
        subpassDesc.pInputAttachments = nullptr;

        // use for multip sampling color attachment (???? look it up)
        subpassDesc.pResolveAttachments = nullptr;

        subpassDesc.preserveAttachmentCount = 0;
        subpassDesc.pPreserveAttachments = nullptr;

        // render pass dependencies , can be multiple dependencies
        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        const size_t attachement_count = 2;
        VkAttachmentDescription attachements[attachement_count] = {
            color_lookup->description,
            depth_lookup->description
        };

        // render pass
        VkRenderPassCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        createInfo.attachmentCount = (uint32_t)attachement_count;
        createInfo.pAttachments = attachements;
        createInfo.subpassCount = 1;
        createInfo.pSubpasses = &subpassDesc;
        createInfo.dependencyCount = 1;
        createInfo.pDependencies = &dependency;
        createInfo.flags = 0;
        createInfo.pNext = 0;

        VkRenderPass renderpass = {};
        VK_CHECK(vkCreateRenderPass(ctx->logical_device_info.handle, &createInfo, ctx->allocator, &renderpass), result);

        if (result != VK_SUCCESS)
        {
            return false;
        }
        
        out_renderpass->internal_data = params;
        out_renderpass->handle = renderpass;
        out_renderpass->begin = Begin;
        out_renderpass->draw = Draw;
        out_renderpass->end = End;
        out_renderpass->on_resize = OnResize;
        out_renderpass->on_destroy = OnDestroy;

        Allocator alloc = Global::alloc_toolbox.heap_allocator;
        
        // create darray of subpasses
        DArray<Subpass>::Create(0 , &out_renderpass->subpasses , alloc );
        
        // create darray of render targets
        DArray<RenderTarget>::Create(ctx->swapchain_info.images_count, &out_renderpass->render_targets, alloc);
        
        // we start creating a renderTarget for each swapchain image
        for (size_t i = 0; i < ctx->swapchain_info.images_count; ++i)
        {
            RenderTarget rt = {};

            // the framebuffer at index [N] has the purpose of providing the needed images for rendering the Nth swapchain image
            // in this simple case , we need two images (aka attachements) (the color , and the depth)
            DArray<VkImageView> attachements = {};
            DArray<VkImageView>::Create(2, &attachements, alloc);
            DArray<VkImageView>::Add(&attachements, ctx->swapchain_info.imageViews.data[i]);
            DArray<VkImageView>::Add(&attachements, ctx->swapchain_info.depthAttachement.view);

            // now we just create the framebuffer with the corresponding attachments
            // those attachement are refered to by index in the renderpass
            // [as specified at the start of this method with the line]
            // "createInfo.pAttachments = attachementDescs.data;"
            FrameBuffer framebuffer = {};
            FrameBuffer::Create(ctx, out_renderpass, ctx->frame_buffer_size, attachements, &framebuffer);

            rt.framebuffer = framebuffer;

            DArray<RenderTarget>::Add(&out_renderpass->render_targets, rt);
        }

        return true;
    }

    static void Draw(Renderpass *in_renderpass, CommandBuffer *cmd, RendererContext *render_ctx)
    {
        
    }

    static void Begin(Renderpass *in_renderpass, CommandBuffer *cmd)
    {
        BasicRenderpassParams *data = (BasicRenderpassParams *)in_renderpass->internal_data;

        VulkanContext *ctx = (VulkanContext *)Global::backend_renderer.user_data;
        uint32_t frame_index = ctx->current_image_index;
        FrameBuffer framebuffer = in_renderpass->render_targets.data[frame_index].framebuffer;

        VkClearValue clear_values[2];
        clear_values[0].color.float32[0] = data->clearColor.r;
        clear_values[0].color.float32[1] = data->clearColor.g;
        clear_values[0].color.float32[2] = data->clearColor.b;
        clear_values[0].color.float32[3] = data->clearColor.a;
        clear_values[1].depthStencil.depth = data->depth;
        clear_values[1].depthStencil.stencil = data->stencil;

        VkRenderPassBeginInfo begin_info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        begin_info.renderPass = in_renderpass->handle;
        begin_info.framebuffer = framebuffer.handle;
        begin_info.renderArea.offset.x = (int32_t)data->area.x;
        begin_info.renderArea.offset.y = (int32_t)data->area.y;
        begin_info.renderArea.extent.width = (int32_t)ctx->frame_buffer_size.x;
        begin_info.renderArea.extent.height = (int32_t)ctx->frame_buffer_size.y;

        begin_info.clearValueCount = 2;
        begin_info.pClearValues = clear_values;

        vkCmdBeginRenderPass(cmd->handle, &begin_info, VK_SUBPASS_CONTENTS_INLINE);

        cmd->state = CommandBufferState::InRenderpass;
    }

    static void OnResize(Renderpass *in_renderpass)
    {
        VulkanContext *ctx = (VulkanContext *)Global::backend_renderer.user_data;
        BasicRenderpassParams *data = (BasicRenderpassParams *)in_renderpass->internal_data;
        Allocator alloc = Global::alloc_toolbox.heap_allocator;

        for (size_t i = 0; i < in_renderpass->render_targets.size; ++i)
        {
            FrameBuffer::Destroy(ctx, &in_renderpass->render_targets.data[i].framebuffer);
        }

        DArray<RenderTarget>::Clear(&in_renderpass->render_targets);

        for (size_t i = 0; i < ctx->swapchain_info.images_count; ++i)
        {
            RenderTarget rt = {};

            DArray<VkImageView> attachements = {};
            DArray<VkImageView>::Create(2, &attachements, alloc);
            DArray<VkImageView>::Add(&attachements, ctx->swapchain_info.imageViews.data[i]);
            DArray<VkImageView>::Add(&attachements, ctx->swapchain_info.depthAttachement.view);

            FrameBuffer framebuffer = {};
            FrameBuffer::Create(ctx, in_renderpass, ctx->frame_buffer_size, attachements, &framebuffer);

            rt.framebuffer = framebuffer;

            DArray<RenderTarget>::Add(&in_renderpass->render_targets, rt);
        }

        data->area.x = 0;
        data->area.y = 0;
        data->area.width = (float)ctx->frame_buffer_size.x;
        data->area.height = (float)ctx->frame_buffer_size.y;
    }

    static void End(Renderpass *in_renderpass, CommandBuffer *cmd)
    {
        vkCmdEndRenderPass(cmd->handle);
        cmd->state = CommandBufferState::Recording;
    }

    static void OnDestroy(Renderpass *in_renderpass)
    {
        VulkanContext *ctx = (VulkanContext *)Global::backend_renderer.user_data;
        BasicRenderpassParams *data = (BasicRenderpassParams *)in_renderpass->internal_data;
        
        for (size_t i = 0; i < in_renderpass->render_targets.size; ++i)
        {
            FrameBuffer::Destroy(ctx, &in_renderpass->render_targets.data[i].framebuffer);
        }

        for(size_t i = 0; i < in_renderpass->subpasses.size; ++i)
        {
            Subpass curr = in_renderpass->subpasses.data[i];
            curr.on_destroy(&curr);
        }

        DArray<RenderTarget>::Destroy(&in_renderpass->render_targets);
        DArray<Subpass>::Destroy(&in_renderpass->subpasses);
    }
};