#pragma once
#include <Containers/DArray.h>
#include "../../Global/Global.h"
#include "../Renderpass/Renderpass.h"
#include "../Context/VulkanContext.h"

struct BasicRenderpassParams
{
    bool sync_window_size;
    Rect area;
    Color clearColor;
    float depth;
    uint32_t stencil;
};

struct BasicRenderpass
{
    static bool Create( VulkanContext* ctx, BasicRenderpassParams params, Renderpass* out_renderpass )
    {
        *out_renderpass = {};

        // main subpass
        VkSubpassDescription subpassDesc = {};
        subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        Allocator heap_alloc = Global::alloc_toolbox.heap_allocator;

        DArray<VkAttachmentDescription> attachementDescs;
        DArray<VkAttachmentDescription>::Create( 2, &attachementDescs, heap_alloc );

        VkAttachmentDescription colorAttachmentDesc = {};
        VkAttachmentReference colorReference = {};

        // color attachement
        {
            colorAttachmentDesc.format = ctx->swapchain_info.surfaceFormat.format;
            colorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
            // loadOp defines what to do to the texture when we load it
            // we can clear OnLoad , keep the previous content , or don't care
            // color
            colorAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

            // storeOp defines what to do to the texture when we store it
            // we can "store it" , (which seems redundant in terms of naming) but means keep the content of this texture to be used by things to come after
            // or don't care , which means that the content wont be used after we're done using it
            colorAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

            // we're not usin stencil for now , so don't care
            // stencil
            colorAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // since this is a brand new "create" , we're not creating this renderpass using an existing image from a previous renderpass , so no "expected" image layout
            colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // what we're gonna transition to after the renderpass , it's a layout format in memory
            colorAttachmentDesc.flags = 0;

            // specifies the
            colorReference.attachment = 0; // attachement index
            colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // layout

            DArray< VkAttachmentDescription>::Add( &attachementDescs, colorAttachmentDesc );
        }

        VkAttachmentDescription depthAttachmentDesc = {};
        VkAttachmentReference depthReference = {};

        // depth attachement
        {
            depthAttachmentDesc.format = ctx->physicalDeviceInfo.depthFormat;
            depthAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;

            // loadOp define what to do to the texture when we load it
            // we can clear OnLoad , keep the previous content , or don't care
            // color
            depthAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

            // storeOp definew what to do to the textre when we store it
            // we can "store i" , which seems redundant but means keep the content of this texture to be used by things to come after
            // or don't care , which means that the content wont be used after we're done using it
            depthAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

            // we're not usin stencil for now , so don't care
            // stencil
            depthAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            depthAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // since this is a brand new "create" , we're not creating this renderpass using an existing image from a previous renderpass , so no "expected" image layout
            depthAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // what we're gonna transition to after the renderpass , it's a layout format in memory
            depthAttachmentDesc.flags = 0;

            // specifies the 
            depthReference.attachment = 1; // attachement index
            depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // layout

            DArray< VkAttachmentDescription>::Add( &attachementDescs, depthAttachmentDesc );
        }

        // todo : we could have other attachement (input , resolve , preserve)

        subpassDesc.colorAttachmentCount = 1;
        subpassDesc.pColorAttachments = &colorReference;
        subpassDesc.pDepthStencilAttachment = &depthReference;

        // input from a shader
        subpassDesc.inputAttachmentCount = 0;
        subpassDesc.pInputAttachments = nullptr;

        // use for multip sampling color attachment (???? look it up)
        subpassDesc.pResolveAttachments = 0;

        subpassDesc.preserveAttachmentCount = 0;
        subpassDesc.pPreserveAttachments = nullptr;

        // render pass dependencies , can be multiple dependencies
        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        // render pass
        VkRenderPassCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        createInfo.attachmentCount = (uint32_t) attachementDescs.size;
        createInfo.pAttachments = attachementDescs.data;
        createInfo.subpassCount = 1;
        createInfo.pSubpasses = &subpassDesc;
        createInfo.dependencyCount = 1;
        createInfo.pDependencies = &dependency;
        createInfo.flags = 0;
        createInfo.pNext = 0;

        VkRenderPass vkRenderpass = {};
        VK_CHECK( vkCreateRenderPass( ctx->logicalDeviceInfo.handle, &createInfo, ctx->allocator, &vkRenderpass ), result );

        if ( result != VK_SUCCESS )
            return false;

        BasicRenderpassParams* data = Global::alloc_toolbox.HeapAlloc<BasicRenderpassParams>();
        data->area = params.area;
        data->clearColor = params.clearColor;
        data->depth = params.depth;
        data->stencil = params.stencil;

        out_renderpass->internal_data = data;
        out_renderpass->handle = vkRenderpass;
        out_renderpass->begin = Begin;
        out_renderpass->end = End;
        out_renderpass->on_resize = OnResize;
        out_renderpass->on_destroy = OnDestroy;
        Allocator alloc = Global::alloc_toolbox.heap_allocator;
        DArray<RenderTarget>::Create( ctx->swapchain_info.imagesCount, &out_renderpass->render_targets, alloc );
        for ( size_t i = 0; i < ctx->swapchain_info.imagesCount; ++i )
        {
            RenderTarget rt = {};

            DArray<VkImageView> attachements = {};
            DArray<VkImageView>::Create( 2, &attachements, alloc );
            DArray<VkImageView>::Add( &attachements, ctx->swapchain_info.imageViews.data[i] );
            DArray<VkImageView>::Add( &attachements, ctx->swapchain_info.depthAttachement.view);

            FrameBuffer framebuffer = {};
            FrameBuffer::Create( ctx, out_renderpass, ctx->frameBufferSize, attachements, &framebuffer );

            rt.framebuffer = framebuffer;

            DArray<RenderTarget>::Add( &out_renderpass->render_targets, rt );
        }      
        
        return true;
    }

    static void Begin( Renderpass* in_renderpass, CommandBuffer* cmd )
    {
        BasicRenderpassParams* data = (BasicRenderpassParams*) in_renderpass->internal_data;

        VulkanContext* ctx = (VulkanContext*) Global::backend_renderer.user_data;
        uint32_t frame_index = ctx->current_image_index;
        FrameBuffer framebuffer = in_renderpass->render_targets.data[frame_index].framebuffer;

        VkRenderPassBeginInfo begin_info = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        begin_info.renderPass = in_renderpass->handle;
        begin_info.framebuffer = framebuffer.handle;

        begin_info.renderArea.offset.x = (int32_t) data->area.x;
        begin_info.renderArea.offset.y = (int32_t) data->area.y;
        begin_info.renderArea.extent.width = (int32_t) ctx->frameBufferSize.x;
        begin_info.renderArea.extent.height = (int32_t) ctx->frameBufferSize.y;

        VkClearValue clear_values[2];

        clear_values[0].color.float32[0] = data->clearColor.r;
        clear_values[0].color.float32[1] = data->clearColor.g;
        clear_values[0].color.float32[2] = data->clearColor.b;
        clear_values[0].color.float32[3] = data->clearColor.a;
        clear_values[1].depthStencil.depth = data->depth;
        clear_values[1].depthStencil.stencil = data->stencil;

        begin_info.clearValueCount = 2;
        begin_info.pClearValues = clear_values;

        vkCmdBeginRenderPass( cmd->handle, &begin_info, VK_SUBPASS_CONTENTS_INLINE );

        cmd->state = CommandBufferState::InRenderpass;
    }

    static void OnResize( Renderpass* in_renderpass )
    {
        VulkanContext* ctx = (VulkanContext*) Global::backend_renderer.user_data;
        BasicRenderpassParams* data = (BasicRenderpassParams*) in_renderpass->internal_data;
        Allocator alloc = Global::alloc_toolbox.heap_allocator;

        for ( size_t i = 0; i < in_renderpass->render_targets.size; ++i )
        {
            FrameBuffer::Destroy( ctx, &in_renderpass->render_targets.data[i].framebuffer);
        }

        DArray<RenderTarget>::Clear( &in_renderpass->render_targets );

        for ( size_t i = 0; i < ctx->swapchain_info.imagesCount; ++i )
        {
            RenderTarget rt = {};

            DArray<VkImageView> attachements = {};
            DArray<VkImageView>::Create( 2, &attachements, alloc );
            DArray<VkImageView>::Add( &attachements, ctx->swapchain_info.imageViews.data[i] );
            DArray<VkImageView>::Add( &attachements, ctx->swapchain_info.depthAttachement.view );

            FrameBuffer framebuffer = {};
            FrameBuffer::Create( ctx, in_renderpass, ctx->frameBufferSize, attachements, &framebuffer );

            rt.framebuffer = framebuffer;

            DArray<RenderTarget>::Add( &in_renderpass->render_targets, rt );
        }

        data->area.x = 0;
        data->area.y = 0;
        data->area.width = (float) ctx->frameBufferSize.x;
        data->area.height = (float) ctx->frameBufferSize.y;
    }

    static void OnDestroy( Renderpass* in_renderpass )
    {
        VulkanContext* ctx = (VulkanContext*) Global::backend_renderer.user_data;

        for ( size_t i = 0; i < in_renderpass->render_targets.size; ++i )
        {
            FrameBuffer::Destroy( ctx, &in_renderpass->render_targets.data[i].framebuffer );
        }

        DArray<RenderTarget>::Destroy(&in_renderpass->render_targets);
    }

    static void End( Renderpass* in_renderpass, CommandBuffer* cmd )
    {
        vkCmdEndRenderPass( cmd->handle );
        cmd->state = CommandBufferState::Recording;
    }
};