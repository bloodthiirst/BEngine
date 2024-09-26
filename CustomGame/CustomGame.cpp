#include <String/StringView.h>
#include <String/StringBuffer.h>
#include <Containers/ArrayView.h>
#include <Maths/Maths.h>
#include <Maths/Color.h>
#include <Typedefs/Typedefs.h>
#include <Core/Defines/Defines.h>
#include <Core/Global/Global.h>
#include <Core/Logger/Logger.h>
#include <Context/CoreContext.h>
#include <Core/Renderer/Context/VulkanContext.h>
#include <Core/Renderer/Context/RendererContext.h>
#include <Core/AssetManager/GlobalAssetManager.h>
#include <Core/AssetManager/MeshAssetManger.h>
#include <Core/Renderer/Mesh/Mesh3D.h>
#include <Core/Renderer/Texture/Texture.h>
#include "CustomGame.h"
#include "SceneCameraController.h"

static const char *game_name = "Custom game title";

struct CustomGameState
{
    SceneCameraController camera_controller;
    Mesh3D plane_mesh;
    ShaderBuilder shader_builder;
    Texture texture;
};

StringView GetName(GameApp *game_app)
{
    return game_name;
}


Texture CreateColorTexture()
{
    VulkanContext* ctx = (VulkanContext*) Global::backend_renderer.user_data;

    size_t width = 64;
    size_t height = 64;
    size_t cell_size = 4;

    TextureDescriptor tex_desc = {};
    tex_desc.create_view = true;
    tex_desc.format = VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;
    tex_desc.height = (uint32_t)height;
    tex_desc.width = (uint32_t)width;
    tex_desc.view_aspect_flags = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
    tex_desc.image_type = VkImageType::VK_IMAGE_TYPE_2D;
    tex_desc.tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
    tex_desc.usage = (VkImageUsageFlagBits)(VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT);

    Texture tex = {};
    Texture::Create(tex_desc, &tex);

    Color *colors = Global::alloc_toolbox.HeapAlloc<Color>(width * height);

    for (size_t y = 0; y < width; ++y)
    {
        for (size_t x = 0; x < width; ++x)
        {
            Color col = {};
            col.r = x / (float) width;
            col.g = y / (float)height;
            col.b = 0;
            col.a = 1;

            colors[x + (y * width)] = col;
        }
    }

    BufferDescriptor desc = {};
    desc.size = (uint32_t)(width * height * sizeof(Color));
    desc.memoryPropertyFlags = (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    desc.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    Buffer copy_buffer = {};
    Buffer::Create(desc, true, &copy_buffer);
    {
        Buffer::Load(0, (uint32_t)(width * height * sizeof(Color)), colors, 0, &copy_buffer);
        Global::alloc_toolbox.HeapFree<Color>(colors);

        CommandBuffer cmd = {};
        VkCommandPool pool = ctx->physicalDeviceInfo.commandPoolsInfo.graphicsCommandPool;
        CommandBuffer::SingleUseAllocateBegin(pool, &cmd);
        Texture::TransitionLayout(&tex, cmd, VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT, VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        Texture::CopyFromBuffer(copy_buffer.handle, &tex, cmd);
        Texture::TransitionLayout(&tex, cmd, VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        CommandBuffer::SingleUseEndSubmit(pool, &cmd, ctx->physicalDeviceInfo.queuesInfo.graphicsQueue);
    }
    Buffer::Destroy(&copy_buffer);

    return tex;
}

Texture CreateGridTexture()
{
    VulkanContext* ctx = (VulkanContext*) Global::backend_renderer.user_data;

    size_t width = 64;
    size_t height = 64;
    size_t cell_size = 4;

    TextureDescriptor tex_desc = {};
    tex_desc.create_view = true;
    tex_desc.format = VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;
    tex_desc.height = (uint32_t)height;
    tex_desc.width = (uint32_t)width;
    tex_desc.view_aspect_flags = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
    tex_desc.image_type = VkImageType::VK_IMAGE_TYPE_2D;
    tex_desc.tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
    tex_desc.usage = (VkImageUsageFlagBits)(VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT);

    Texture tex = {};
    Texture::Create(tex_desc, &tex);

    Color *colors = Global::alloc_toolbox.HeapAlloc<Color>(width * height);

    Color dark = {1, 1, 1, 1};
    Color light = {0, 0, 0, 1};

    for (size_t y = 0; y < width; ++y)
    {
        bool is_y_odd = (y / cell_size) % 2;

        for (size_t x = 0; x < width; ++x)
        {
            bool is_x_odd = (x / cell_size) % 2;

            bool col = is_x_odd ^ is_y_odd;

            colors[x + (y * width)] = col ? dark : light;
        }
    }

    BufferDescriptor desc = {};
    desc.size = (uint32_t)(width * height * sizeof(Color));
    desc.memoryPropertyFlags = (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    desc.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    Buffer copy_buffer = {};
    Buffer::Create(desc, true, &copy_buffer);
    {
        Buffer::Load(0, (uint32_t)(width * height * sizeof(Color)), colors, 0, &copy_buffer);
        Global::alloc_toolbox.HeapFree<Color>(colors);

        CommandBuffer cmd = {};
        VkCommandPool pool = ctx->physicalDeviceInfo.commandPoolsInfo.graphicsCommandPool;
        CommandBuffer::SingleUseAllocateBegin(pool, &cmd);
        Texture::TransitionLayout(&tex, cmd, VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT, VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        Texture::CopyFromBuffer(copy_buffer.handle, &tex, cmd);
        Texture::TransitionLayout(&tex, cmd, VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        CommandBuffer::SingleUseEndSubmit(pool, &cmd, ctx->physicalDeviceInfo.queuesInfo.graphicsQueue);
    }
    Buffer::Destroy(&copy_buffer);

    return tex;
}

ShaderBuilder CreateShaderBuilder()
{
    Allocator alloc = Global::alloc_toolbox.heap_allocator;

    StringView vert_path = "C:\\Dev\\BEngine\\BEngine\\Core\\Resources\\SimpleShader.vert.spv";
    StringView frag_path = "C:\\Dev\\BEngine\\BEngine\\Core\\Resources\\SimpleShader.frag.spv";

    FileHandle vert_handle = {};
    FileHandle frag_handle = {};

    if (!Global::platform.filesystem.open(vert_path, FileModeFlag::Read, true, &vert_handle))
    {
        Global::logger.Error("Can't find vertex shader");
        return {};
    }

    if (!Global::platform.filesystem.open(frag_path, FileModeFlag::Read, true, &frag_handle))
    {
        Global::logger.Error("Can't find frag shader");
        return {};
    }

    size_t vert_size = {};
    size_t frag_size = {};

    Global::platform.filesystem.get_size(&vert_handle, &vert_size);
    Global::platform.filesystem.get_size(&frag_handle, &frag_size);

    StringBuffer vert_code = StringBuffer::Create(vert_size, alloc);
    StringBuffer frag_code = StringBuffer::Create(frag_size, alloc);

    size_t bytes_read = {};
    Global::platform.filesystem.read_all(vert_handle, vert_code.buffer, &bytes_read);
    Global::platform.filesystem.read_all(frag_handle, frag_code.buffer, &bytes_read);

    ShaderBuilder builder = ShaderBuilder::Create()
                                .SetName("Basic_Textured")
                                .SetStage(VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, vert_code)
                                .SetStage(VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT, frag_code)
                                .AddVertexAttribute("position", 0, sizeof(Vector3), VkFormat::VK_FORMAT_R32G32B32_SFLOAT)
                                .AddVertexAttribute("texcoord", 1, sizeof(Vector2), VkFormat::VK_FORMAT_R32G32_SFLOAT)
                                .AddDescriptor("global_ubo", 0, 0, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT)
                                .AddDescriptor("diffuse_sampler", 1, 0, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);

    if (vert_handle.is_valid)
    {
        Global::platform.filesystem.close(&vert_handle);
    }

    if (frag_handle.is_valid)
    {
        Global::platform.filesystem.close(&frag_handle);
    }

    return builder;
}

Mesh3D CreatePlane()
{
    const uint32_t vert_count = 4;
    const uint32_t index_count = 6;

    const Vector2 pos = Vector2(0 , 0);
    const Vector2 size = Vector2(500 , 300);

    const Vector2 tr = pos + size;
    const Vector2 tl = pos + Vector2(0 , size.y);
    const Vector2 br = pos + Vector2(size.x , 0);
    const Vector2 bl = pos;

    Vertex3D vertPositions[vert_count] = 
    {
        {tr , Vector2(1.0f, 1.0f)}, // TOP RIGHT
        {tl , Vector2(0.0f, 1.0f)}, // TOP LEFT
        {br , Vector2(1.0f, 0.0f)}, // BOT RIGHT
        {bl , Vector2(0.0f, 0.0f)}  // BOT LEFT
    }; 

    uint32_t vertIndicies[6] = {
        2,
        1,
        0,

        2,
        3,
        1,
    };

    Mesh3D plane_mesh = {};
    ArrayView<Vertex3D> vert_view = {};
    vert_view.data = vertPositions;
    vert_view.size = vert_count;

    ArrayView<uint32_t> index_view = {};
    index_view.data = vertIndicies;
    index_view.size = index_count;

    Mesh3D::Create(&plane_mesh, vert_view, index_view, Global::alloc_toolbox.heap_allocator);
    
    return plane_mesh;
}

void Initialize(GameApp *game_app)
{
    // CoreContext::DefaultContext();

    game_app->game_state.camera_position = Vector3(0, 0, -3);
    game_app->game_state.camera_rotation = {1, 0, 0, 0};

    CustomGameState *state = Global::alloc_toolbox.HeapAlloc<CustomGameState>();
    game_app->user_data = state;

    // init camera
    {
        state->camera_controller.position = Vector3(0, 0, -3);
        state->camera_controller.xRotation = 0;
        state->camera_controller.yRotation = 0;
    }

    // create assets
    {
        state->plane_mesh = CreatePlane();
        state->shader_builder = CreateShaderBuilder();
        state->texture = CreateColorTexture();
    }
}

void OnUpdate(GameApp *game_app, float delta_time)
{
    CustomGameState *state = (CustomGameState *)game_app->user_data;

    Vector3 new_pos = {};
    Quaternion new_rot = {};

    state->camera_controller.Tick(delta_time, &new_pos, &new_rot);

    game_app->game_state.camera_position = new_pos;
    game_app->game_state.camera_rotation = new_rot;
}

void OnRender(GameApp *game_app, RendererContext *render_ctx, float delta_time)
{
    CustomGameState *state = (CustomGameState *)game_app->user_data;

    DrawMesh draw = {};
    draw.mesh = &state->plane_mesh;
    draw.shader_builder = &state->shader_builder;
    draw.texture = &state->texture;

    DArray<DrawMesh>::Add(&render_ctx->mesh_draws, draw);
}

void Destroy(GameApp *game_app)
{
    CustomGameState *state = (CustomGameState *)game_app->user_data;
    
    Mesh3D::Destroy(&state->plane_mesh);
    Texture::Destroy(&state->texture);
    ShaderBuilder::Destroy(&state->shader_builder);

    Global::alloc_toolbox.HeapFree((CustomGameState *)game_app->user_data);
}

extern "C" __declspec(dllexport) GameApp GetGameApp()
{
    GameApp game = {};

    game.get_name = GetName;
    game.initialize = Initialize;
    game.on_update = OnUpdate;
    game.on_render = OnRender;
    game.destroy = Destroy;

    game.game_startup.x = 250;
    game.game_startup.y = 300;
    game.game_startup.width = 500;
    game.game_startup.height = 500;

    return game;
}
