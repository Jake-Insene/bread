#pragma once
#include "graphics/gles/gles_driver.h"

using GLID = u32;

struct GLESMemoryAllocator
{
    struct GLESBuffer
    {
        ResourceID self_id;
        GLID buffer;
        usize size;
        GLenum target;
        GLenum usage;
    };

    struct GLESTexture
    {
        Image* image;
        ResourceID self_id;
        GLID texture;

        // texture
        Vector2I size;
        GLenum target;
        GLenum internal_format;
        GLenum texture_format;
        GLenum min_filter;
        GLenum mag_filter;
    };

    struct GLESRenderTarget
    {
        ResourceID self_id;
        GLID framebuffer;
        GLID color_buffer;
        GLenum format;
        Vector2I size;
    };

    struct InternalData
    {
        mem::Allocator allocator;

        usize allocated_bytes;
        usize free_bytes;

        Array<GLESBuffer> buffers;
        Array<GLESTexture> textures;
        Array<GLESRenderTarget> render_targets;
    };

    static inline InternalData data;

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

    // Allocation/deallocation tools
    static GLESBuffer& buffer_allocate();
    static void buffer_free(ResourceID rid);
    static GLID buffer_allocate_handle();
    static void buffer_deallocate_handle(GLID buffer, usize size);
    static void buffer_fill_memory(GLID buffer, Slice<const u8> mem, GLenum target, GLenum usage);
    static void buffer_update_memory(GLID buffer, usize offset, Slice<const u8> mem, GLenum target);
    static void buffer_bind_and_update_memory(GLID buffer, usize offset, Slice<const u8> mem, GLenum target);

    static GLID buffer_allocate_handle_and_fill(GLESBuffer& buffer, Slice<const u8> mem, 
        GLenum target, GLenum usage);

    static GLESTexture& texture_allocate();
    static void texture_free(ResourceID rid);
    static GLID texture_allocate_handle();
    static ResourceID allocate_texture_from_info(const TextureCreateInfo& create_info);
    static GLID texture_allocate_handle_and_fill(GLESTexture& texture, const TextureCreateInfo& create_info);
    // Pixels being null is valid
    static void texture_allocate_memory(GLID texture, GLenum target, const Vector2I& size,
        GLenum internal_format, GLenum input_format, Slice<u8> bytes);

    static GLESRenderTarget& render_target_allocate();
    static void render_target_free(ResourceID rid);
    static GLID render_target_allocate_handle();
    static ResourceID allocate_render_target_from_info(const RenderTargetCreateInfo& create_info);

    static void render_target_bind_texture(GLID render_target, GLID texture);
    
    // Buffer
    static GLESBuffer& buffer_get(ResourceID rid);
    
    // Texture
    static GLESTexture& texture_get(ResourceID rid);
    static void texture_set_image(ResourceID rid, Image* image);
    static Vector2I texture_get_size(ResourceID rid);
    static GLID texture_get_handle(ResourceID rid);

    static void texture_filter(GLID texture, GLenum target, GLenum min, GLenum mag);
    static void texture_wrap(GLID texture, GLenum target, GLenum wrap);
    

    // Render Target
    static GLESRenderTarget& render_target_get(ResourceID rid);

    static Vector2I render_target_get_size(ResourceID rid);
    static void render_target_set_size(ResourceID rid, const Vector2I& new_size);
    static GLID render_target_get_handle(ResourceID rid);

};

