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

    struct VTable
    {
        VTFunc(void, initialize, const mem::Allocator&);
        VTFunc(void, shutdown);

        VTFunc(void, recreate);
        VTFunc(void, destroy);
        
        VTFunc(void, render);
        VTFunc(void, present);
        
        VTFunc(void, add_cmd, const RenderCommand&);
        
        VTFunc(ResourceID, texture_create, const TextureCreateInfo&);
        VTFunc(void, texture_set_image, ResourceID, Image*);
        VTFunc(Vector2I, texture_get_size, ResourceID);
        
        VTFunc(ResourceID, render_target_create, const RenderTargetCreateInfo&);
        VTFunc(Vector2I, render_target_get_size, ResourceID);
        VTFunc(void, render_target_set_size, ResourceID, const Vector2I&);
    };
    
    static inline VTable vtable;
    
    static void initialize(const mem::Allocator& allocator, DriverType driver);
    VTFuncDefS(shutdown);
    
    VTFuncDefS(recreate);
    VTFuncDefS(destroy);
    
    VTFuncDefS(render);
    VTFuncDefS(present);
    
    VTFuncDefArg1S(add_cmd, const RenderCommand&);
    
    // Texture
    VTFuncDefArg1RetS(ResourceID, texture_create, const TextureCreateInfo&);
    VTFuncDefArg2S(texture_set_image, ResourceID, Image*);
    VTFuncDefArg1RetS(Vector2I, texture_get_size, ResourceID);
    
    // Render Target
    VTFuncDefArg1RetS(ResourceID, render_target_create, const RenderTargetCreateInfo&);
    VTFuncDefArg1RetS(Vector2I, render_target_get_size, ResourceID);
    VTFuncDefArg2S(render_target_set_size, ResourceID, const Vector2I&);
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
