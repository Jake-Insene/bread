#pragma once
#include "core/header.h"
#include "mem/allocator.h"
#include "graphics/structs.h"
#include "graphics/command_interface.h"
#include "math/vec2.h"

struct Image;

struct Graphics
{
    enum DriverType
    {
        DRIVER_UNKNOWN = 0,

        GLES,

        DEFAULT_DRIVER = GLES,
    };

    using TextureID = ID<u32>;
    using RenderTargetID = ID<u32>;

    struct RenderInfo
    {
        Color clear_color;
    };
    
    struct VTable
    {
        VTFunc(void, initialize, const mem::Allocator&);
        VTFunc(void, shutdown);

        VTFunc(void, recreate);
        VTFunc(void, destroy);
        
        VTFunc(void, render, RenderTargetID, const RenderInfo&);
        VTFunc(void, present, RenderTargetID);
        
        VTFunc(void, add_cmd, const RenderCommand&);
        
        VTFunc(TextureID, create_texture, const TextureCreateInfo&);
        VTFunc(void, destroy_texture, TextureID);
        VTFunc(RenderTargetID, create_render_target, const RenderTargetCreateInfo&);
        VTFunc(void, destroy_render_target, RenderTargetID);

        // Texture
        VTFunc(void, texture_set_image, TextureID, Image*);
        VTFunc(Vector2I, texture_get_size, TextureID);
        
        // Render Target
        VTFunc(void, render_target_set_size, RenderTargetID, const Vector2I&);
        VTFunc(Vector2I, render_target_get_size, RenderTargetID);
    };
    
#if ENABLE_GRAPHICS_DRIVERS
    static inline VTable vtable;
    
    static void initialize(const mem::Allocator& allocator, DriverType driver);
    VTFuncDefS(shutdown);
    
    VTFuncDefS(recreate);
    VTFuncDefS(destroy);
    
    VTFuncDefArg2S(render, RenderTargetID, const RenderInfo& ri);
    VTFuncDefArg1S(present, RenderTargetID);
    
    VTFuncDefArg1S(add_cmd, const RenderCommand&);
    
    VTFuncDefArg1RetS(TextureID, create_texture, const TextureCreateInfo&);
    VTFuncDefArg1S(destroy_texture, TextureID);
    VTFuncDefArg1RetS(RenderTargetID, create_render_target, const RenderTargetCreateInfo&);
    VTFuncDefArg1S(destroy_render_target, RenderTargetID);

    VTFuncDefArg2S(texture_set_image, TextureID, Image*);
    VTFuncDefArg1RetS(Vector2I, texture_get_size, TextureID);
    
    VTFuncDefArg2S(render_target_set_size, RenderTargetID, const Vector2I&);
    VTFuncDefArg1RetS(Vector2I, render_target_get_size, RenderTargetID);
#else

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

    static void recreate();
    static void destroy();

    static void render(RenderTargetID rt_id, const RenderInfo& ri);
    static void present(RenderTargetID rt_id);

    static void add_cmd(const RenderCommand& cmd);
    
    static TextureID create_texture(const TextureCreateInfo& create_info);
    static void destroy_texture(TextureID tex_id);
    static RenderTargetID create_render_target(const RenderTargetCreateInfo& create_info);
    static void destroy_render_target(RenderTargetID rt_id);

    static void texture_set_image(TextureID tex_id, Image* img);
    static Vector2I texture_get_size(TextureID tex_id);

    static void render_target_set_size(RenderTargetID rt_id, const Vector2I& new_size);
    static Vector2I render_target_get_size(RenderTargetID rt_id);

#endif

    static void draw_texture(const Transform2D& transform, const Vector2& dest_extent, const Rect2D& src_rect,
        TextureID texture_id, Color mod_color, RenderCommand::BatchFlags flags
    );

    static void draw_canvas_element(const Transform2D& transform, const Vector2& dest_extent, const Rect2D& src_rect,
        TextureID texture_id, Color mod_color, RenderCommand::BatchFlags flags
    );

    static void draw_quad(const Transform2D& transform, const Vector2& size, const Color& color);
    static void draw_line(const Vector2& start, const Vector2& end, Color color);
    static void draw_circle(const Vector2& point, f32 radius, Color color);
};

