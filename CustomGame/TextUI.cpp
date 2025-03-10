#include "TextUI.h"
#include "EntryPoint.h"

TextUI TextUI::Create(StringView text)
{
    VulkanContext *ctx = (VulkanContext *)Global::backend_renderer.user_data;

    const size_t size_for_text = sizeof(TextCharData) * text.length;

    TextUI txt = {};
    txt.text = text;
    FreeList::AllocBlock(&ctx->descriptors_freelist, size_for_text, &txt.instance_matricies);

    return txt;
}

void TextUI::Destroy(TextUI *txt)
{
    VulkanContext *ctx = (VulkanContext *)Global::backend_renderer.user_data;
    FreeList::FreeBlock(&ctx->descriptors_freelist, txt->instance_matricies);

    *txt = {};
}

DrawMesh TextUI::GetDraw()
{
    VulkanContext *ctx = (VulkanContext *)Global::backend_renderer.user_data;
    EntryPoint *entry = (EntryPoint *)Global::app.game_app.user_data;

    DArray<TextCharData> char_data = {};
    DArray<TextCharData>::Create(text.length, &char_data, Global::alloc_toolbox.frame_allocator);

    FontInfo font = entry->font_info;

    Vector2 offset = { 0, 50};
    for (size_t i = 0; i < text.length; ++i)
    {
        char c = text.buffer[i];
        CharacterInfo char_info = font.char_info_lookup.data[c];
 
        float line_offset = char_info.bearing_y - char_info.character_height;
        float starting_space = char_info.bearing_x;
        float ending_space = char_info.advance - starting_space - char_info.character_width;

        Matrix4x4 char_mat = Matrix4x4(
            {char_info.character_width, 0, 0, offset.x + starting_space},
            {0, char_info.character_height , 0, offset.y + line_offset},
            {0, 0, 1, 0},
            {0, 0, 0, 1});

        if(c == ' ')
        {
            offset.x += font.descriptor.font_size_px * 0.5f;
        }
        else
        {
            offset.x += char_info.character_width + ending_space;
        }
        TextCharData data = {};
        data.uv_rect = char_info.uv_rect;
        data.quad_matrix = char_mat;

        DArray<TextCharData>::Add(&char_data, data);
    }

    const size_t size_for_text = sizeof(TextCharData) * text.length;

    DrawMesh draw = {};
    draw.instances_count = text.length;
    draw.instances_data = instance_matricies;
    draw.mesh = &entry->plane_mesh;
    draw.shader_builder = &entry->text_shader_builder;
    draw.texture = &entry->font_info.font_atlas_texture;

    VkCommandPool pool = ctx->physical_device_info.command_pools_info.graphicsCommandPool;
    VkQueue queue = ctx->physical_device_info.queues_info.graphics_queue;
    Buffer::Load(0, size_for_text, char_data.data, 0, &ctx->staging_buffer);
    Buffer::Copy(pool, {}, queue, &ctx->staging_buffer, 0, &ctx->descriptors_buffer, instance_matricies.start, size_for_text);

    return draw;
}