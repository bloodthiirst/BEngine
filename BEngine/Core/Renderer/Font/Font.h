#include <Containers/ArrayView.h>
#include <Containers/DArray.h>
#include <ft2build.h>
#include "../../Renderer/Texture/Texture.h"
#include FT_FREETYPE_H
#include "../../Global/Global.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../Utils/stb_image_writer.h"

struct FontDescriptor
{
    size_t font_size_px;
    Vector2Int atlas_size;
};

struct Font
{
    FT_Face face;
    DArray<char> font_ttf_buffer;

    static void Destroy(Font *inout_font)
    {
        FT_Done_Face(inout_font->face);
        DArray<char>::Destroy(&inout_font->font_ttf_buffer);

        *inout_font = {};
    }

    void static MyDrawBitmap(FT_Bitmap *bitmap, uint8_t *dst, Vector2Int size)
    {
        /* for simplicity, we assume that `bitmap->pixel_mode' */
        /* is `FT_PIXEL_MODE_GRAY' (i.e., not a bitmap font)   */

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
                dst[texture_idx] = bitmap->buffer[glyph_idx];
            }
        }
    }

    static bool GenerateAtlas(Font *in_font, FontDescriptor in_atlas, Texture *out_tex)
    {
        FT_Error error = {};

        Texture atlas = {};
        TextureDescriptor desc = {};
        desc.create_view = true;
        desc.format = VkFormat::VK_FORMAT_R32_SFLOAT;
        desc.image_type = VkImageType::VK_IMAGE_TYPE_2D;
        desc.memory_flags = 0;
        desc.tiling = VkImageTiling::VK_IMAGE_TILING_LINEAR;
        desc.usage = 0;
        desc.view_aspect_flags = 0;
        desc.height = in_atlas.atlas_size.x;
        desc.width = in_atlas.atlas_size.y;

        // Texture::Create(desc, &atlas);
        ArenaCheckpoint check = Global::alloc_toolbox.GetArenaCheckpoint();
        {
            const size_t channel_number = 1;
            const size_t texture_size = sizeof(uint8_t) * in_atlas.atlas_size.x * in_atlas.atlas_size.y * channel_number;
            uint8_t *texture_buffer = (uint8_t *)ALLOC(Global::alloc_toolbox.frame_allocator, texture_size);
            Global::platform.memory.mem_set(texture_buffer ,255, texture_size);
            
            error = FT_Set_Pixel_Sizes(
                in_font->face,          /* handle to face object */
                0,                      /* pixel_width           */
                in_atlas.font_size_px); /* pixel_height          */

            assert(error == FT_Err_Ok);

            FT_GlyphSlot slot = in_font->face->glyph;

            size_t x_offset = 0;

            for (char c = 'a'; c <= 'z'; c++)
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

                uint8_t *start_index = texture_buffer + x_offset;

                // note : here we insturct freeType to draw inside the buffer
                // pass the bottom-left point to start drawing from
                MyDrawBitmap(&slot->bitmap, start_index, in_atlas.atlas_size);

                x_offset += slot->bitmap.width;
            }

            stbi_write_jpg("Atlas.jpg", in_atlas.atlas_size.x, in_atlas.atlas_size.y, 1, texture_buffer , 100);    
            stbi_write_png("Atlas.png", in_atlas.atlas_size.x, in_atlas.atlas_size.y, 1, texture_buffer , sizeof(uint8_t) * in_atlas.atlas_size.x);
        }
        Global::alloc_toolbox.ResetArenaOffset(&check);
        return true;
    }
};

struct FontImporter
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