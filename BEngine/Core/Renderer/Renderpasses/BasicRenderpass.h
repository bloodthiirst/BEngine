#pragma once
#include <Containers/DArray.h>
#include "../../Global/Global.h"
#include "../Renderpass/Renderpass.h"
#include "../CommandBuffer/CommandBuffer.h"
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
        out_renderpass->draw = Draw;
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
            DArray<VkImageView>::Add( &attachements, ctx->swapchain_info.depthAttachement.view );

            FrameBuffer framebuffer = {};
            FrameBuffer::Create( ctx, out_renderpass, ctx->frameBufferSize, attachements, &framebuffer );

            rt.framebuffer = framebuffer;

            DArray<RenderTarget>::Add( &out_renderpass->render_targets, rt );
        }

        return true;
    }

    static bool UpdateTexture( VulkanContext* context, Shader* in_shader )
    {
        uint32_t currentIndex = context->current_image_index;
        CommandBuffer currentCmdBuffer = context->swapchain_info.graphics_cmd_buffers_per_image.data[currentIndex];
        VkDescriptorSet currentDescriptor = in_shader->descriptor_sets[currentIndex].data[1];

        VkDescriptorImageInfo image_info = {};
        image_info.imageView = context->default_texture.view;
        image_info.sampler = context->default_sampler;
        image_info.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet writeDescriptor = {};
        writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptor.dstSet = currentDescriptor;   // "dst" stands for "descriptor" here 
        writeDescriptor.dstBinding = 0;               // "dst" stands for "descriptor" here
        writeDescriptor.dstArrayElement = 0;          // "dst" stands for "descriptor" here
        writeDescriptor.descriptorCount = 1;
        writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptor.pImageInfo = &image_info;

        vkUpdateDescriptorSets( context->logicalDeviceInfo.handle, 1, &writeDescriptor, 0, nullptr );

        return true;
    }

    static bool UpdateGlobalState( BackendRenderer* in_backend, Matrix4x4 projection, Matrix4x4 view, float ambiant, uint32_t mode )
    {
        VulkanContext* ctx = (VulkanContext*) in_backend->user_data;

        CommandBuffer current_cmd = ctx->swapchain_info.graphics_cmd_buffers_per_image.data[ctx->current_image_index];

        // NOTE : the projection and view are passed by copy because the frame might not be done
        ctx->default_shader.global_UBO.projection = projection;
        ctx->default_shader.global_UBO.view = view;

        Shader::UpdateGlobalBuffer( ctx, ctx->default_shader.global_UBO, &ctx->default_shader );

        return true;
    }

    static void Draw( Renderpass* in_renderpass, CommandBuffer* cmd, RendererContext* render_ctx )
    {
        BasicRenderpassParams* data = (BasicRenderpassParams*) in_renderpass->internal_data;
        BackendRenderer* in_backend = &Global::backend_renderer;
        VulkanContext* ctx = (VulkanContext*) Global::backend_renderer.user_data;

        uint32_t frame_index = ctx->current_image_index;
        FrameBuffer framebuffer = in_renderpass->render_targets.data[frame_index].framebuffer;

        // test code
        {
            Shader::Bind( ctx, &ctx->default_shader );

            GameState* state = &Global::app.game_app.game_state;

            float aspect = (float) Global::platform.window.height / Global::platform.window.width;
            Matrix4x4 proj = Matrix4x4::Perspective( 60, 0.1f, 100, aspect );

            Matrix4x4 corrective_mat = Matrix4x4( { 1,0,0,0 }, { 0,-1,0,0 }, { 0,0,0,0 }, { 0,0,0,1 } );
            Matrix4x4 tra_mat = Matrix4x4::Translate( state->camera_position );
            Matrix4x4 rot_mat = Matrix4x4::Rotate( state->camera_rotation );
            Matrix4x4 scl_mat = Matrix4x4::Scale( Vector3( 1, 1, -1 ) );
            Matrix4x4 nonInvView = tra_mat * rot_mat;
            Matrix4x4 view = Matrix4x4::Inverse( nonInvView ) * scl_mat;

            {
                // vulkan considers (0,0) to be the upper-left corner
                // to get "standanrdized" zero point , we set the the center to be (bottom-left)
                // hence why the y == height and height = -height
                VkViewport viewport = {};
                viewport.x = 0;
                viewport.y = (float) ctx->frameBufferSize.y;
                viewport.width = (float) ctx->frameBufferSize.x;
                viewport.height = -(float) ctx->frameBufferSize.y;
                viewport.maxDepth = 1;
                viewport.minDepth = 0;

                VkRect2D scissor = { };
                scissor.offset.x = 0;
                scissor.offset.y = 0;
                scissor.extent.width = ctx->frameBufferSize.x;
                scissor.extent.height = ctx->frameBufferSize.y;

                vkCmdSetViewport( cmd->handle, 0, 1, &viewport );
                vkCmdSetScissor( cmd->handle, 0, 1, &scissor );
            }

            UpdateTexture( ctx, &ctx->default_shader );
            UpdateGlobalState( in_backend, proj, view, 1, 0 );

            DArray<VkDescriptorSet>* currSet = &ctx->default_shader.descriptor_sets[frame_index];
            for ( size_t i = 0; i < currSet->size; ++i )
            {
                VkDescriptorSet* curr = &currSet->data[i];
                vkCmdBindDescriptorSets( cmd->handle, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->default_shader.pipeline.layout, (uint32_t) i, 1, curr, 0, nullptr );
            }

            VkDeviceSize pos_offsets[1] = { 0 };
            VkDeviceSize tex_offsets[1] = { 12 };
            vkCmdBindVertexBuffers( cmd->handle, 0, 1, &ctx->vertexBuffer.handle, (VkDeviceSize*) pos_offsets );
            vkCmdBindVertexBuffers( cmd->handle, 1, 1, &ctx->vertexBuffer.handle, (VkDeviceSize*) tex_offsets );
            vkCmdBindIndexBuffer( cmd->handle, ctx->indexBuffer.handle, 0, VkIndexType::VK_INDEX_TYPE_UINT32 );
            vkCmdDrawIndexed( cmd->handle, 6, 1, 0, 0, 0 );
        }
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
            FrameBuffer::Destroy( ctx, &in_renderpass->render_targets.data[i].framebuffer );
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

    static void End( Renderpass* in_renderpass, CommandBuffer* cmd )
    {
        vkCmdEndRenderPass( cmd->handle );
        cmd->state = CommandBufferState::Recording;
    }

    static void OnDestroy( Renderpass* in_renderpass )
    {
        VulkanContext* ctx = (VulkanContext*) Global::backend_renderer.user_data;

        for ( size_t i = 0; i < in_renderpass->render_targets.size; ++i )
        {
            FrameBuffer::Destroy( ctx, &in_renderpass->render_targets.data[i].framebuffer );
        }

        DArray<RenderTarget>::Destroy( &in_renderpass->render_targets );
    }
};