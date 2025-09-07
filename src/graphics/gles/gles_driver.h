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
    };
    
    static inline InternalData data;

    static Graphics::VTable get_vtable();

    [[nodiscard]] static mem::Allocator& get_allocator() { return data.allocator; }

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

    static void recreate();
    static void destroy();
    
    static void render(Graphics::RenderTargetID rt_id, const Graphics::RenderInfo& ri);
    static void present(Graphics::RenderTargetID rt_id);
    
    static void add_cmd(const RenderCommand& cmd);

    static Graphics::TextureID create_texture(const TextureCreateInfo& create_info);
    static void destroy_texture(Graphics::TextureID);
    static Graphics::RenderTargetID create_render_target(const RenderTargetCreateInfo& create_info);
    static void destroy_render_target(Graphics::RenderTargetID);

    static void texture_set_image(Graphics::TextureID tex_id, Image* img);
    static Vector2I texture_get_size(Graphics::TextureID tex_id);

    static void render_target_set_size(Graphics::RenderTargetID rt_id, const Vector2I& new_size);
    static Vector2I render_target_get_size(Graphics::RenderTargetID rt_id);

    static void _init_context();
};

