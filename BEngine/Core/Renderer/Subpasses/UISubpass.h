#pragma once
#include <Containers/DArray.h>
#include <Containers/HMap.h>
#include "../../Global/Global.h"
#include "../Subpasses/Subpass.h"
#include "../CommandBuffer/CommandBuffer.h"
#include "../Buffer/Buffer.h"
#include "../Context/VulkanContext.h"

struct UISubpassParams
{
    bool sync_window_size;
    HMap<ShaderBuilder , Shader> shader_lookup;
    Buffer camera_matrix_buffer;
};

struct UISubpass
{
    static inline const char *BASIC_RENDERPASS_ID = "UI Subpass - ID";

    static bool Create(VulkanContext *ctx, Subpass *out_subpass)
    {
        *out_subpass = {};
        out_subpass->id = StringView::Create(BASIC_RENDERPASS_ID);

        UISubpassParams *data = Global::alloc_toolbox.HeapAlloc<UISubpassParams>();

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

        out_subpass->internal_data = data;
        out_subpass->begin = Begin;
        out_subpass->draw = Draw;
        out_subpass->end = End;
        out_subpass->on_resize = OnResize;
        out_subpass->on_destroy = OnDestroy;

        return true;
    }

    static void Draw(Subpass *in_subpass, CommandBuffer *cmd, RendererContext *render_ctx)
    {
        UISubpassParams *data = (UISubpassParams *)in_subpass->internal_data;
        BackendRenderer *in_backend = &Global::backend_renderer;
        VulkanContext *ctx = (VulkanContext *)Global::backend_renderer.user_data;

        uint32_t frame_index = ctx->current_image_index;

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

            Renderpass* renderpass = &in_subpass->graph->renderpasses.data[in_subpass->renderpass_index];

            VkDeviceSize pos_offsets[1] = {0};
            VkDeviceSize tex_offsets[1] = {12};

            for (size_t i = 0; i < render_ctx->mesh_draws.size; ++i)
            {
                DrawMesh curr = render_ctx->mesh_draws.data[i];

                Shader* shader_ptr = {};
                Shader shader = {};
                
                if(!HMap<ShaderBuilder,Shader>::TryGet(&data->shader_lookup , *curr.shader_builder , &shader_ptr))
                {
                    bool build = curr.shader_builder->Build(ctx , renderpass , &shader);
                    assert(build);

                    HMap<ShaderBuilder,Shader>::TryAdd(&data->shader_lookup , *curr.shader_builder , shader , nullptr);
                }
                else
                {
                    shader = *shader_ptr;
                }
                
                assert(shader.pipeline.handle != VK_NULL_HANDLE);

                Shader::Bind(ctx, &shader);
                Shader::SetBuffer(ctx, &shader, 0, &data->camera_matrix_buffer , 0 , sizeof(GlobalUniformObject));
                Shader::SetBuffer(ctx , &shader,2, &ctx->descriptors_buffer , curr.instances_data.start , curr.instances_data.size);
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

    static void Begin(Subpass *in_subpass, CommandBuffer *cmd) { }

    static void OnResize(Subpass *in_subpass) { }

    static void End(Subpass *in_subpass, CommandBuffer *cmd) { }

    static void OnDestroy(Subpass *in_subpass)
    {
        VulkanContext *ctx = (VulkanContext *)Global::backend_renderer.user_data;
        UISubpassParams *data = (UISubpassParams *)in_subpass->internal_data;

        Buffer::Destroy(&data->camera_matrix_buffer);
        
        ArenaCheckpoint check = Global::alloc_toolbox.GetArenaCheckpoint();
        {
            DArray<Pair<ShaderBuilder, Shader>> kv_vals = {};
            DArray<Pair<ShaderBuilder, Shader>>::Create(data->shader_lookup.count , &kv_vals , Global::alloc_toolbox.frame_allocator);
            HMap<ShaderBuilder,Shader>::GetAll(&data->shader_lookup , &kv_vals);

            for(size_t i = 0; i < kv_vals.size ; ++i)
            {
                Pair<ShaderBuilder, Shader> curr = kv_vals.data[i];
                Shader::Destroy(ctx , &curr.value);
            }

            HMap<ShaderBuilder,Shader>::Destroy(&data->shader_lookup);
        }
        Global::alloc_toolbox.ResetArenaOffset(&check);

        Global::alloc_toolbox.HeapFree<UISubpassParams>(data);
    }
};