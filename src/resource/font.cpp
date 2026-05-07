#include "resource/font.h"

#include "engine/engine.h"
#include "render_device/render_device.h"
#include "resource/resource_manager.h"
#include "resource/resource_manager_internal.h"
#include "engine/engine.h"

#include <external/stb_truetype.h>


void flip_atlas_vertical(const mem::Allocator& allocator, Slice<u8> pixels, i32 width, i32 height)
{
    const i32 row_size = width; // R8

    Slice<u8> tmp = allocator.alloc(row_size, 16);

    for (i32 y = 0; y < height / 2; ++y)
    {
        Slice<u8> row_top    = pixels.add(y * row_size);
        Slice<u8> row_bottom = pixels.add((height - 1 - y) * row_size);

        mem::copy(tmp,       row_top.slice(row_size));
        mem::copy(row_top,    row_bottom.slice(row_size));
        mem::copy(row_bottom, tmp.slice(row_size));
    }

    allocator.free(tmp);
}


static void _load_theme(const mem::Allocator& allocator, const Slice<u8>& font_file_content, stbtt_fontinfo* font, Font::FontTheme& theme)
{
    stbtt_pack_context pack_context;
    stbtt_packedchar ranges[Font::MinimumGlyphCount] = {};
    
    bool success = false;
    i32 width = 512;
    while(success == false)
    {
        Slice<u8> pixels = allocator.alloc(width * width, 16);

        stbtt_PackBegin(&pack_context, pixels.ptr(), width, width, width, 0, 0);

        i32 result = stbtt_PackFontRange(
            &pack_context, font_file_content.ptr(), 0, 
            f32(theme.font_size), 0, Font::MinimumGlyphCount, ranges
        );
        if(result != 1)
        {
            width *= 2;
            stbtt_PackEnd(&pack_context);
            allocator.free(pixels);
            continue;
        }
        else
        {
            success = true;  
        }

        for (u32 glyph_index = 0; glyph_index < Font::MinimumGlyphCount; glyph_index++)
        {
            Font::Glyph& glyph = theme.glyphs.get(glyph_index);

            glyph.advance.width = ranges[glyph_index].xadvance;

            i32 ascent;
            i32 descent;
            i32 line_gap;
            stbtt_GetFontVMetrics(font, &ascent, &descent, &line_gap);

            f32 scale = stbtt_ScaleForPixelHeight(font, f32(theme.font_size));
            f32 line_advance = (ascent - descent + line_gap) * scale;
            glyph.advance.height = line_advance;
            
            f32 u0 = ranges[glyph_index].x0;
            f32 v0 = ranges[glyph_index].y0;
            f32 u1 = ranges[glyph_index].x1;
            f32 v1 = ranges[glyph_index].y1;
            
            f32 glyph_width = f32(u1 - u0);
            f32 glyph_height = f32(v1 - v0);

            // Conversion of the top-left coordinate system to bottom-left coordinate system
            f32 glyph_atlas_pos_x = u0;
            f32 glyph_atlas_pos_y = width - v0 - glyph_height;

            glyph.src_rect = Rect2D(
                Vector2(glyph_atlas_pos_x, glyph_atlas_pos_y),
                Vector2(glyph_width, glyph_height)
            );
        }

        flip_atlas_vertical(allocator, pixels, width, width);
        theme.font_atlas = Engine::get_render_device()->get_gpu_resource_manager()->create_texture(
            {
                .type = GPU::TextureType::Texture2D,
                .format = GPU::TextureFormat::R8Srgb,
                .extent = Vector3U(width, width, 1),
                .pixels = pixels,
            }
        );

        stbtt_PackEnd(&pack_context);
        allocator.free(pixels);
    }
}


void Font::init(const ResourceCreateInfo& info)
{
    Resource::init(info);

    data.themes = Array<FontTheme>::with_allocator(allocator);
}


void Font::destroy()
{
    for (FontTheme& theme : data.themes.iter())
    {
        if (theme.font_atlas == GPUTextureID::invalid())
        {
            continue;
        }
    
        Engine::get_render_device()->get_gpu_resource_manager()->destroy_texture(theme.font_atlas);
        
        theme.glyphs.destroy();
    }

    data.themes.destroy();
    Resource::destroy();
}


Error Font::load(StringView file_path)
{
    if (!File::exists(file_path))
    {
        RMDebugInfo("Couldn't load the font '{}'", file_path);
        return MakeError(ErrorCode::FileNotFound);
    }

    path.set(file_path);

    Slice<u8> content = File::read_all(allocator, file_path);

    stbtt_fontinfo font;
    stbtt_InitFont(&font, content.ptr(), stbtt_GetFontOffsetForIndex(content.ptr(), 0));

    FontTheme& default_theme = data.themes.add(FontTheme());
    default_theme.glyphs = Array<Glyph>::with_allocator(allocator);
    default_theme.font_size = DefaultFontSize;

    default_theme.glyphs.resize(MinimumGlyphCount);
    _load_theme(allocator, content, &font, default_theme);
    
    allocator.free(content);

    return ErrorCode::Ok;
}

const Font::FontTheme& Font::get_font_theme(i32 font_size)
{
    for (auto& theme : data.themes.iter())
    {
        if (theme.font_size == font_size)
            return theme;
    }

    return _theme_with_size(font_size);
}

const Font::FontTheme& Font::_theme_with_size(i32 font_size)
{
    Slice<u8> content = File::read_all(allocator, path.view());

    stbtt_fontinfo font;
    stbtt_InitFont(&font, content.ptr(), stbtt_GetFontOffsetForIndex(content.ptr(), 0));

    FontTheme& new_theme = data.themes.add(FontTheme());
    new_theme.glyphs = Array<Glyph>::with_allocator(allocator);
    new_theme.font_size = font_size;

    new_theme.glyphs.resize(MinimumGlyphCount);
    _load_theme(allocator, content, &font, new_theme);

    allocator.free(content);

    return new_theme;
}
