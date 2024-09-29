#pragma once
#include <Containers/DArray.h>
#include <Containers/HMap.h>
#include "../../Global/Global.h"
#include "../Renderpass/Renderpass.h"
#include "../CommandBuffer/CommandBuffer.h"
#include "../Buffer/Buffer.h"
#include "../Context/VulkanContext.h"

struct UIRenderpassParams
{
    bool sync_window_size;
    Rect area;
    Color clearColor;
    float depth;
    uint32_t stencil;

    HMap<ShaderBuilder , Shader> shader_lookup;
    Buffer camera_matrix_buffer;
};


struct UIRenderpass
{
    static inline const char *BASIC_RENDERPASS_ID = "BasicRenderPass - ID";

    static bool Create(VulkanContext *ctx, UIRenderpassParams params, Renderpass *out_renderpass)
    {
        *out_renderpass = {};

        out_renderpass->id = StringView::Create(BASIC_RENDERPASS_ID);

        // main subpass
        VkSubpassDescription subpassDesc = {};
        subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        Allocator heap_alloc = Global::alloc_toolbox.heap_allocator;

        DArray<VkAttachmentDescription> attachementDescs;
        DArray<VkAttachmentDescription>::Create(2, &attachementDescs, heap_alloc);

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

            colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;     // since this is a brand new "create" , we're not creating this renderpass using an existing image from a previous renderpass , so no "expected" image layout
            colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // what we're gonna transition to after the renderpass , it's a layout format in memory
            colorAttachmentDesc.flags = 0;

            // specifies the :
            colorReference.attachment = 0;                                    // attachement index
            colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // layout

            DArray<VkAttachmentDescription>::Add(&attachementDescs, colorAttachmentDesc);
        }

        VkAttachmentDescription depthAttachmentDesc = {};
        VkAttachmentReference depthReference = {};

        // depth attachement
        {
            depthAttachmentDesc.format = ctx->physical_device_info.depthFormat;
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

            depthAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                      // since this is a brand new "create" , we're not creating this renderpass using an existing image from a previous renderpass , so no "expected" image layout
            depthAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // what we're gonna transition to after the renderpass , it's a layout format in memory
            depthAttachmentDesc.flags = 0;

            // specifies the
            depthReference.attachment = 1;                                            // attachement index
            depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // layout

            DArray<VkAttachmentDescription>::Add(&attachementDescs, depthAttachmentDesc);
        }

        // todo : we could have other attachement (input , resolve , preserve)

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

        // render pass
        VkRenderPassCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        createInfo.attachmentCount = (uint32_t)attachementDescs.size;
        createInfo.pAttachments = attachementDescs.data;
        createInfo.subpassCount = 1;
        createInfo.pSubpasses = &subpassDesc;
        createInfo.dependencyCount = 1;
        createInfo.pDependencies = &dependency;
        createInfo.flags = 0;
        createInfo.pNext = 0;

        VkRenderPass vkRenderpass = {};
        VK_CHECK(vkCreateRenderPass(ctx->logical_device_info.handle, &createInfo, ctx->allocator, &vkRenderpass), result);

        if (result != VK_SUCCESS)
        {
            return false;
        }

        BasicRenderpassParams *data = Global::alloc_toolbox.HeapAlloc<BasicRenderpassParams>();

        data->area = params.area;
        data->clearColor = params.clearColor;
        data->depth = params.depth;
        data->stencil = params.stencil;
        data->camera_matrix_buffer;
        HMap<ShaderBuilder,Shader>::Create(&data->shader_lookup , Global::alloc_toolbox.heap_allocator , 10 , 10 , ShaderUtils::ShaderBuilderHash , ShaderUtils::ShaderBuilderCmp );

        // camera buffer
        {
            BufferDescriptor desc = {};
            desc.sharing_mode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
            desc.size = sizeof(GlobalUniformObject);
            desc.memoryPropertyFlags = (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            desc.usage = (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

            Buffer::Create(desc, true, &data->camera_matrix_buffer);
        }

        out_renderpass->internal_data = data;
        out_renderpass->handle = vkRenderpass;
        out_renderpass->begin = Begin;
        out_renderpass->draw = Draw;
        out_renderpass->end = End;
        out_renderpass->on_resize = OnResize;
        out_renderpass->on_destroy = OnDestroy;

        Allocator alloc = Global::alloc_toolbox.heap_allocator;
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
        UIRenderpassParams *data = (UIRenderpassParams *)in_renderpass->internal_data;
        BackendRenderer *in_backend = &Global::backend_renderer;
        VulkanContext *ctx = (VulkanContext *)Global::backend_renderer.user_data;

        uint32_t frame_index = ctx->current_image_index;
        FrameBuffer framebuffer = in_renderpass->render_targets.data[frame_index].framebuffer;

        // Render logic
        {
            GameState *state = &Global::app.game_app.game_state;
            float screen_w = Global::platform.window.width;
            float screen_h = Global::platform.window.height;
            float aspect = screen_h / screen_w;
            
            Matrix4x4 proj = Matrix4x4(
                { 2 / screen_w  , 0             , 0 , -1},
                { 0             , 2 / screen_h  , 0 , -1},                
                { 0             , 0             , 2 , +0},
                { 0             , 0             , 0 , +1}
                );
            
            Matrix4x4 view = Matrix4x4::Identity();

            // viewport setup
            {
                // vulkan considers (0,0) to be the upper-left corner
                // to get "standanrdized" zero point , we set the the center to be (bottom-left)
                // hence why the y == height and height = -height
                VkViewport viewport = {};
                viewport.x = 0;
                viewport.y = (float)ctx->frame_buffer_size.y;
                viewport.width = (float)ctx->frame_buffer_size.x;
                viewport.height = -(float)ctx->frame_buffer_size.y;
                viewport.maxDepth = 1;
                viewport.minDepth = 0;

                VkRect2D scissor = {};
                scissor.offset.x = 0;
                scissor.offset.y = 0;
                scissor.extent.width = ctx->frame_buffer_size.x;
                scissor.extent.height = ctx->frame_buffer_size.y;

                vkCmdSetViewport(cmd->handle, 0, 1, &viewport);
                vkCmdSetScissor(cmd->handle, 0, 1, &scissor);
            }

            // load View and Projection matrices
            {
                GlobalUniformObject guo = {};
                guo.projection = proj;
                guo.view = view;
                guo.time = (float) Global::platform.time.get_system_time(&Global::platform.time);

                Buffer::Load(0, sizeof(GlobalUniformObject), &guo, 0, &data->camera_matrix_buffer);
            }

            VkDeviceSize pos_offsets[1] = {0};
            VkDeviceSize tex_offsets[1] = {12};

            for (size_t i = 0; i < render_ctx->mesh_draws.size; ++i)
            {
                DrawMesh curr = render_ctx->mesh_draws.data[i];

                Shader* shader_ptr = {};
                Shader shader = {};
                
                if(!HMap<ShaderBuilder,Shader>::TryGet(&data->shader_lookup , *curr.shader_builder , &shader_ptr))
                {
                    bool build = curr.shader_builder->Build(ctx , in_renderpass , &shader);
                    assert(build);

                    HMap<ShaderBuilder,Shader>::TryAdd(&data->shader_lookup , *curr.shader_builder , shader , nullptr);
                }
                else
                {
                    shader = *shader_ptr;
                }
                
                assert(shader.pipeline.handle != VK_NULL_HANDLE);

                Shader::Bind(ctx, &shader);
                Shader::SetBuffer(ctx, &shader, 0, &data->camera_matrix_buffer);
                Shader::SetBuffer(ctx , &shader,2, &curr.instances_data);
                Shader::SetTexture(ctx, &shader, 1, curr.texture);

                DArray<VkDescriptorSet> curr_set = shader.descriptor_sets[frame_index];

                for (size_t i = 0; i < curr_set.size; ++i)
                {
                    VkDescriptorSet *currSet = &curr_set.data[i];
                    
                    vkCmdBindDescriptorSets(cmd->handle, VK_PIPELINE_BIND_POINT_GRAPHICS, shader.pipeline.layout, (uint32_t)i, 1, currSet, 0, nullptr);
                }

                VkDeviceSize vert_offset = (pos_offsets[0]) + curr.mesh->verticies_block.start;
                // bind the vertex positions , since they are in the first field , they have an offset of 0
                vkCmdBindVertexBuffers(cmd->handle, 0, 1, &ctx->mesh_buffer.handle, &vert_offset);

                VkDeviceSize uv_offset = (tex_offsets[0]) + curr.mesh->verticies_block.start;
                // bind the vertex UVs , since they are right after the position (vec3) , they have an offset of 12 ( 12 == sizeof(Vertex3D.position))
                vkCmdBindVertexBuffers(cmd->handle, 1, 1, &ctx->mesh_buffer.handle, &uv_offset);

                // bind mesh indices
                vkCmdBindIndexBuffer(cmd->handle, ctx->mesh_buffer.handle, curr.mesh->indicies_block.start, VkIndexType::VK_INDEX_TYPE_UINT32);

                // finally issue the draw command
                vkCmdDrawIndexed(cmd->handle, curr.mesh->indicies.size, curr.instances_count, 0, 0, 0);

                pos_offsets[i] += sizeof(Vertex3D);
                tex_offsets[i] += sizeof(Vertex3D);
            }
        }
    }

    static void Begin(Renderpass *in_renderpass, CommandBuffer *cmd)
    {
        UIRenderpassParams *data = (UIRenderpassParams *)in_renderpass->internal_data;

        VulkanContext *ctx = (VulkanContext *)Global::backend_renderer.user_data;
        uint32_t frame_index = ctx->current_image_index;
        FrameBuffer framebuffer = in_renderpass->render_targets.data[frame_index].framebuffer;

        VkRenderPassBeginInfo begin_info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        begin_info.renderPass = in_renderpass->handle;
        begin_info.framebuffer = framebuffer.handle;

        begin_info.renderArea.offset.x = (int32_t)data->area.x;
        begin_info.renderArea.offset.y = (int32_t)data->area.y;
        begin_info.renderArea.extent.width = (int32_t)ctx->frame_buffer_size.x;
        begin_info.renderArea.extent.height = (int32_t)ctx->frame_buffer_size.y;

        VkClearValue clear_values[2];

        clear_values[0].color.float32[0] = data->clearColor.r;
        clear_values[0].color.float32[1] = data->clearColor.g;
        clear_values[0].color.float32[2] = data->clearColor.b;
        clear_values[0].color.float32[3] = data->clearColor.a;
        clear_values[1].depthStencil.depth = data->depth;
        clear_values[1].depthStencil.stencil = data->stencil;

        begin_info.clearValueCount = 2;
        begin_info.pClearValues = clear_values;

        vkCmdBeginRenderPass(cmd->handle, &begin_info, VK_SUBPASS_CONTENTS_INLINE);

        cmd->state = CommandBufferState::InRenderpass;
    }

    static void OnResize(Renderpass *in_renderpass)
    {
        VulkanContext *ctx = (VulkanContext *)Global::backend_renderer.user_data;
        UIRenderpassParams *data = (UIRenderpassParams *)in_renderpass->internal_data;
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

        ArenaCheckpoint check = Global::alloc_toolbox.GetArenaCheckpoint();
        
        DArray<Pair<ShaderBuilder, Shader>> kv_vals = {};
        DArray<Pair<ShaderBuilder, Shader>>::Create(data->shader_lookup.count , &kv_vals , Global::alloc_toolbox.frame_allocator);
        HMap<ShaderBuilder,Shader>::GetAll(&data->shader_lookup , &kv_vals);

        for(size_t i = 0; i < kv_vals.size ; ++i)
        {
            Pair<ShaderBuilder, Shader> curr = kv_vals.data[i];
            Shader::Destroy(ctx , &curr.value);
        }

        HMap<ShaderBuilder,Shader>::Destroy(&data->shader_lookup);
        Buffer::Destroy(&data->camera_matrix_buffer);

        for (size_t i = 0; i < in_renderpass->render_targets.size; ++i)
        {
            FrameBuffer::Destroy(ctx, &in_renderpass->render_targets.data[i].framebuffer);
        }

        Global::alloc_toolbox.ResetArenaOffset(&check);
        DArray<RenderTarget>::Destroy(&in_renderpass->render_targets);
    }
};