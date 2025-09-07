#pragma once
#include "collections/array.h"
#include "graphics/graphics.h"
#include "platform/platform_header.h"
#include "resource/image.h"

#define GLESDebugInfo(...) DebugInfo("[GLESDriver]: " __VA_ARGS__)


struct GLESDriver
{   
    struct GLLimits
    {
        i32 max_texture_units;
    };

    struct InternalData
    {
        mem::Allocator allocator;
        
        GLLimits limits;
        Vector2I current_viewport_size;
    };
    
    static inline InternalData data;

    [[nodiscard]] static mem::Allocator& get_allocator() { return data.allocator; }
    [[nodiscard]] static Vector2I get_current_viewport_size() { return data.current_viewport_size; }

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

    static void recreate();
    static void destroy();
    
    static void render(Viewport* viewport);
    static void present(Viewport* viewport);
    
    static TextureID create_texture(const TextureCreateInfo& create_info);
    static void destroy_texture(TextureID tex_id);
    static RenderTargetID create_render_target(const RenderTargetCreateInfo& create_info);
    static void destroy_render_target(RenderTargetID rt_id);

    static RenderTargetID get_main_render_target();

    static void texture_set_image(TextureID tex_id, Image* img);
    static Vector2I texture_get_size(TextureID tex_id);

    static void render_target_set_size(RenderTargetID rt_id, const Vector2I& new_size);
    static Vector2I render_target_get_size(RenderTargetID rt_id);

    static void _init_context();
};

