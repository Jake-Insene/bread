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

    struct VTable
    {
        VTFunc(void, initialize, const mem::Allocator&);
        VTFunc(void, shutdown);

        VTFunc(void, recreate);
        VTFunc(void, destroy);
        
        VTFunc(void, render);
        VTFunc(void, present);
        
        VTFunc(void, add_cmd, const RenderCommand&);
        
        VTFunc(TextureID, create_texture, const TextureCreateInfo&);
        VTFunc(void, destroy_texture, TextureID);
        VTFunc(RenderTargetID, create_render_target, const RenderTargetCreateInfo&);
        VTFunc(void, destroy_render_target, RenderTargetID);

        VTFunc(void, texture_set_image, TextureID, Image*);
        VTFunc(Vector2I, texture_get_size, TextureID);
        
        VTFunc(Vector2I, render_target_get_size, RenderTargetID);
        VTFunc(void, render_target_set_size, RenderTargetID, const Vector2I&);
    };
    
    static inline VTable vtable;
    
    static void initialize(const mem::Allocator& allocator, DriverType driver);
    VTFuncDefS(shutdown);
    
    VTFuncDefS(recreate);
    VTFuncDefS(destroy);
    
    VTFuncDefS(render);
    VTFuncDefS(present);
    
    VTFuncDefArg1S(add_cmd, const RenderCommand&);
    
    VTFuncDefArg1RetS(TextureID, create_texture, const TextureCreateInfo&);
    VTFuncDefArg1S(destroy_texture, TextureID);
    VTFuncDefArg1RetS(RenderTargetID, create_render_target, const RenderTargetCreateInfo&);
    VTFuncDefArg1S(destroy_render_target, RenderTargetID);

    // Texture
    VTFuncDefArg2S(texture_set_image, TextureID, Image*);
    VTFuncDefArg1RetS(Vector2I, texture_get_size, TextureID);
    
    // Render Target
    VTFuncDefArg1RetS(Vector2I, render_target_get_size, RenderTargetID);
    VTFuncDefArg2S(render_target_set_size, RenderTargetID, const Vector2I&);
};

namespace Graphics2D
{
    
inline void draw_quad(const Color& color, const Vector2& size, const Transform2D& transform)
{
    Graphics::add_cmd(
        RenderCommand
        {
            .type = RenderCommand::DRAW_QUAD,
            .quad =
            {
                .transform = transform,
                .size = size,
                .color = color,
            }
        }
    );
}

inline void draw_line(Color color, Vector2 start, Vector2 end)
{
    Graphics::add_cmd(
        RenderCommand
        {
            .type = RenderCommand::DRAW_LINE,
            .line =
            {
                .start = start,
                .end = end,
                .color = color,
            }
        }
    );
}

inline void draw_texture(const Transform2D& transform, Vector2 texture_extent, Vector2 dest_extent, const Rect2D& src_rect, 
    ResourceID texture_id, Color mod_color, RenderCommand::SpriteFlags flags)
{
    Graphics::add_cmd(
        RenderCommand
        {
            .type = RenderCommand::DRAW_SPRITE,
            .sprite =
            {
                .transform = transform,
                .texture_extent = texture_extent,
                .dest_extent = dest_extent,
                .src_rect = src_rect,
                .texture = texture_id,
                .color = mod_color,
                .flags = flags,
            },
        }
        );
}

inline void draw_canvas_element(const Transform2D& transform, Vector2 texture_extent, Vector2 dest_extent, const Rect2D& src_rect,
    ResourceID texture_id, Color mod_color, RenderCommand::CanvasFlags flags)
{
    Graphics::add_cmd(
        RenderCommand
        {
            .type = RenderCommand::DRAW_CANVAS_ELEMENT,
            .canvas_element =
            {
                .transform = transform,
                .texture_extent = texture_extent,
                .dest_extent = dest_extent,
                .src_rect = src_rect,
                .texture = texture_id,
                .color = mod_color,
                .flags = flags,
            },
        }
    );
}

}
