#pragma once
#include "core/array.h"
#include "graphics/graphics.h"
#include "io/image.h"
#include "platform/platform_header.h"

struct GLESDriver
{

    struct GLESTexture
    {
        Image* image;
        ResourceID self_id;
        u32 gl_id;
        
        // texture
        Vector2I size;
        i32 gl_target;
        i32 gl_internal_format;
        i32 gl_format;
        i32 gl_min_filter;
        i32 gl_mag_filter;
    };
    
    struct GLESRenderTarget
    {
        ResourceID self_id;
        u32 gl_id;
        
        u32 color_buffer;
        
        Vector2I size;
    };
    
    struct GLLimits
    {
        u32 max_texture_units;
    };
    
    struct InternalData
    {
        mem::Allocator allocator;
        
        GLLimits limits;
        Array<GLESTexture> textures;
        Array<GLESRenderTarget> render_targets;
    };
    
    static inline InternalData data;;

    static Graphics::VTable get_vtable();

    [[nodiscard]] static mem::Allocator& get_allocator() { return data.allocator; }

    static void initialize(mem::Allocator& allocator);
    static void shutdown();
    static void recreate();
    static void destroy();
    static void init_context();
    static void destroy_context();
    
    static void render();
    static void present();
    
    static void add_cmd(const RenderCommand& cmd);
    
    // Textures
    static ResourceID texture_create(const TextureCreateInfo& create_info);
    static void texture_set_image(ResourceID tid, Image* image);
    static Vector2I texture_get_size(ResourceID tid);

    // RenderTarget
    static ResourceID render_target_create(const RenderTargetCreateInfo& create_info);
    static Vector2I render_target_get_size(ResourceID rtid);
    
    // pixels being null is valid
    static u32 make_texture_and_fill(i32 minf, i32 magf, i32 wrap, Vector2I extent, i32 internal_format, void* pixels, i32 input_format, i32 target);
    
    static GLESTexture& textures_allocate();
    static GLESTexture& textures_get(ResourceID rid);
    
    static GLESRenderTarget& render_targets_allocate();
    static GLESRenderTarget& render_targets_get(ResourceID rid);
};

