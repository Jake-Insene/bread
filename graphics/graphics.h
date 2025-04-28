#pragma once
#include "core/header.h"
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
        VTFunc(void, initialize, mem::Allocator&);
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
    };
    
    static inline VTable vtable;
    
    static void initialize(mem::Allocator& allocator, DriverType driver);
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
}
