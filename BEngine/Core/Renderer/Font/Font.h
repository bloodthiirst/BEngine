#pragma once
#include <Containers/ArrayView.h>
#include <Containers/DArray.h>
#include <ft2build.h>
#include "../../Utils/stb_image_writer.h"
#include "../../Renderer/Context/VulkanContext.h"
#include "../../Renderer/Texture/Texture.h"
#include FT_FREETYPE_H
#include "../../Global/Global.h"


struct BAPI FontDescriptor
{
    size_t font_size_px;
    Vector2Int atlas_size;
};

struct BAPI CharacterInfo
{
    char characeter;
    Rect tex_rect;
    Rect uv_rect;
    float character_width;
    float character_height;
    float bearing_y;
    float bearing_x;
    float advance;
};

struct BAPI FontInfo
{
    FontDescriptor descriptor;
    Texture font_atlas_texture;
    DArray<CharacterInfo> char_info_lookup;
};

struct BAPI Font
{
    FT_Face face;
    DArray<char> font_ttf_buffer;

    static void Destroy(Font *inout_font)
    {
        FT_Done_Face(inout_font->face);
        DArray<char>::Destroy(&inout_font->font_ttf_buffer);

        *inout_font = {};
    }

    void static WriteToTexture(FT_Bitmap *bitmap, uint8_t *texture_dst, size_t char_index , FontDescriptor desc , Vector2* out_uv_start)
    {
        /* for simplicity, we assume that `bitmap->pixel_mode' */
        /* is `FT_PIXEL_MODE_GRAY' (i.e., not a bitmap font)   */

        char c = (char) char_index;
        Vector2Int size = desc.atlas_size;

        size_t cells_per_row = desc.atlas_size.x / desc.font_size_px;
        size_t cells_per_col = desc.atlas_size.y / desc.font_size_px;
        
        size_t y_offset = (char_index / cells_per_row);
        size_t x_offset = char_index - (y_offset * cells_per_row);

        out_uv_start->x = x_offset * desc.font_size_px;
        out_uv_start->y = (y_offset * desc.font_size_px);

        y_offset = cells_per_col - y_offset;
        
        uint8_t* texture_start = texture_dst + (y_offset * desc.atlas_size.x * desc.font_size_px) + (x_offset * desc.font_size_px);

        for (int32_t y = 0; y < bitmap->rows; y++)
        {
            for (int32_t x = 0; x < bitmap->width; x++)
            {
                bool oob = (x < 0 || y < 0) || (x >= size.x || y >= size.y);

                if(oob)
                {
                    continue;
                }

                int32_t glyph_idx = (y * bitmap->width) + x;
                int32_t texture_idx = (y * size.x) + x;

                uint8_t val = bitmap->buffer[glyph_idx];
                texture_start[texture_idx] = val;
            }
        }

    }

    struct CustomSTBIContext
    {
        size_t length;
        uint8_t** png_ptr;
    };

    static void custom_stbi_write_mem(void *context, void *data, int size) 
    {
        CustomSTBIContext *c = (CustomSTBIContext*)context; 
        uint8_t* mem = (uint8_t*) ALLOC(Global::alloc_toolbox.frame_allocator , size);
        c->length = (size_t)size;
        
        Global::platform.memory.mem_copy(data , mem , size);
        *c->png_ptr = mem;
    }

    static bool GenerateAtlas(Font *in_font, FontDescriptor in_desc, FontInfo *out_info)
    {
        *out_info = {};
        DArray<CharacterInfo>::Create(255 , &out_info->char_info_lookup , Global::alloc_toolbox.heap_allocator);
        
        size_t row_size = in_desc.atlas_size.x / in_desc.font_size_px;
        size_t col_size = in_desc.atlas_size.y / in_desc.font_size_px;
        size_t total_cells = row_size * col_size;

        assert(total_cells >= 255);

        FT_Error error = {};

        VkFormat fmt = VkFormat::VK_FORMAT_R8_UNORM;

        Texture atlas_texture = {};
        TextureDescriptor desc = {};
        desc.create_view = true;
        desc.mipmaps_level = 1;
        desc.format = fmt;
        desc.image_type = VkImageType::VK_IMAGE_TYPE_2D;
        desc.memory_flags = 0;
        desc.view_aspect_flags = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
        desc.tiling = VkImageTiling::VK_IMAGE_TILING_LINEAR;
        desc.height = in_desc.atlas_size.x;
        desc.width = in_desc.atlas_size.y;
        desc.usage = (VkImageUsageFlagBits)(VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 
                                            VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
                                            VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT);
        Texture::Create(desc, &atlas_texture);
        
        ArenaCheckpoint check = Global::alloc_toolbox.GetArenaCheckpoint();
        {
            const size_t channel_number = 1;
            const size_t texture_size = sizeof(uint8_t) * in_desc.atlas_size.x * in_desc.atlas_size.y * channel_number;
            uint8_t *texture_buffer = (uint8_t *)ALLOC(Global::alloc_toolbox.frame_allocator, texture_size);
            Global::platform.memory.mem_set(texture_buffer ,0, texture_size);
            
            error = FT_Set_Pixel_Sizes(
                in_font->face,          /* handle to face object */
                in_desc.font_size_px,  /* pixel_width           */
                in_desc.font_size_px); /* pixel_height          */

            assert(error == FT_Err_Ok);

            FT_GlyphSlot slot = in_font->face->glyph;

            size_t x_offset = 0;

            for (size_t c = 0; c < 255; c++)
            {
                uint32_t glyph_index = FT_Get_Char_Index(in_font->face, c);
                int32_t load_flags = FT_LOAD_DEFAULT;

                error = FT_Load_Glyph(
                    in_font->face, /* handle to face object */
                    glyph_index,   /* glyph index           */
                    load_flags);   /* load flags, see below */

                assert(error == FT_Err_Ok);

                FT_Render_Mode render_mode = FT_Render_Mode::FT_RENDER_MODE_NORMAL;
                error = FT_Render_Glyph(in_font->face->glyph, /* glyph slot  */
                                        render_mode);         /* render mode */

                assert(error == FT_Err_Ok);
                
                // note : here we insturct freeType to draw inside the buffer
                // pass the top-left point to start drawing from
                // then returned UV point is bottom-left

                // note : the metrics are expressed in 1/64s of a pixel
                // so shifting to the right by 6 is equivalent of dividing by 64
                Rect tex_rect = {};
                tex_rect.size = { (float) (slot->metrics.width >> 6), (float)(slot->metrics.height >> 6) };
                WriteToTexture(&slot->bitmap, texture_buffer, (size_t) c, in_desc , &tex_rect.pos);

                Rect uv_rect = tex_rect;
                uv_rect.width /= in_desc.atlas_size.x;
                uv_rect.height /= in_desc.atlas_size.y;
                uv_rect.x /= in_desc.atlas_size.x;
                uv_rect.y /= in_desc.atlas_size.y;

                CharacterInfo char_info = {};
                char_info.characeter = (char) c;
                char_info.tex_rect = tex_rect;
                char_info.uv_rect = uv_rect;
                char_info.character_width = (float) (slot->metrics.width >> 6);
                char_info.character_height = (float) (slot->metrics.height >> 6);
                char_info.advance = (float) (slot->metrics.horiAdvance >> 6);
                char_info.bearing_x = (float) (slot->metrics.horiBearingX >> 6);
                char_info.bearing_y = (float) (slot->metrics.horiBearingY >> 6);               

                DArray<CharacterInfo>::Add(&out_info->char_info_lookup , char_info);
            }

            StringView atlas_path = StringUtils::Format( Global::alloc_toolbox.frame_allocator, "{}\\{}.png",Global::app.application_startup.executable_folder , in_font->face->family_name).view;

            // NOTE : this memory will point to the frame_arena , so no need to free it manually
            uint8_t* png_mem = {};

            CustomSTBIContext context = {};
            context.length = 0;
            context.png_ptr = &png_mem;

            int result = stbi_write_png_to_func(custom_stbi_write_mem , &context , in_desc.atlas_size.x , in_desc.atlas_size.y , 1  , texture_buffer , sizeof(uint8_t) * in_desc.atlas_size.x );
            assert(result == 1);

            FileHandle file_h = {};
            bool open = Global::platform.filesystem.open(atlas_path , FileModeFlag::ReadWrite , true , &file_h);
            assert(open);

            ArrayView<char> view = { (char*) png_mem , context.length };
            bool write = Global::platform.filesystem.write_bytes(&file_h , view);
            assert(write);
            Global::platform.filesystem.close(&file_h);

            // upload to texture
            {
                VulkanContext* ctx = (VulkanContext*) Global::backend_renderer.user_data;
                Buffer::Load(0, (uint32_t)(in_desc.atlas_size.x * in_desc.atlas_size.y * sizeof(uint8_t)), texture_buffer, 0, &ctx->staging_buffer);
                CommandBuffer cmd = {};
                VkCommandPool pool = ctx->physical_device_info.command_pools_info.graphicsCommandPool;
                CommandBuffer::SingleUseAllocateBegin(pool, &cmd);
                Texture::TransitionLayout(&atlas_texture, cmd, fmt, VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                Texture::CopyFromBuffer(ctx->staging_buffer.handle, &atlas_texture, cmd);
                Texture::TransitionLayout(&atlas_texture, cmd, fmt, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                CommandBuffer::SingleUseEndSubmit(pool, &cmd, ctx->physical_device_info.queues_info.graphics_queue);
            }

            out_info->descriptor = in_desc;
            out_info->font_atlas_texture = atlas_texture;
            FREE(Global::alloc_toolbox.frame_allocator , texture_buffer);
        }
        
        Global::alloc_toolbox.ResetArenaOffset(&check);
        return true;
    }
};

struct BAPI FontImporter
{
    FT_Library library;

    static bool Create(FontImporter *out_font_importer)
    {
        FT_Library library; /* handle to library     */
        FT_Face face;       /* handle to face object */

        *out_font_importer = {};

        FT_Error error = FT_Init_FreeType(&library);
        if (error != FT_Err_Ok)
        {
            return false;
        }

        out_font_importer->library = library;
        return true;
    }

    static void Destroy(FontImporter *inout_font_importer)
    {
        FT_Done_FreeType(inout_font_importer->library);
        *inout_font_importer = {};
    }

    static bool LoadFont(FontImporter *in_font_importer, ArrayView<char> in_ttf_buffer, Font *out_font)
    {
        DArray<char> buffer = {};
        DArray<char>::Create(in_ttf_buffer.data, &buffer, in_ttf_buffer.size, Global::alloc_toolbox.heap_allocator);

        const unsigned char *data = (unsigned char *)buffer.data;
        FT_Face face = {};
        FT_Error error = {};

        error = FT_New_Memory_Face(in_font_importer->library,
                                   data,        /* first byte in memory */
                                   buffer.size, /* size in bytes        */
                                   0,           /* face_index           */
                                   &face);

        *out_font = {};

        if (error != FT_Err_Ok)
        {
            DArray<char>::Destroy(&buffer);
            return false;
        }

        out_font->font_ttf_buffer = buffer;
        out_font->face = face;

        return true;
    }
};