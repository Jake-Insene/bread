#pragma once
#include "core/header.h"
#include "io/resource_id.h"
#include "math/mat4.h"
#include "math/transform_2d.h"
#include "math/rect_2d.h"
#include "math/color.h"

struct RenderCommand
{
    enum SpriteFlags
    {
        FLAG_NONE = 0x0,
        FLAG_TOP_LEFT = 0x1,
        FLAG_FLIP_V = 0x2,
        FLAG_FLIP_H = 0x4,
    };

    enum CommandType
    {
        NONE = 0,
        BIND_RENDER_TARGET,
        CLEAR_RENDER_TARGET,
        
        DRAW_SPRITE,
        DRAW_QUAD,
        DRAW_LINE,
    } type;
    
    union
    {
        struct
        {
            ResourceID source_id;
        } bind;
        
        struct
        {
            Transform2D transform;
            // The size of the rectangle where the texture will be draw.
            Vector2 texture_extent;
            // In texture.
            Rect2D src_rect;
            ResourceID texture;
            Color color;
            SpriteFlags flags;
        } sprite;
        
        struct
        {
            ResourceID rid;
            Color color;
            // TODO: add depth
        } clear;
        
        struct
        {
            Transform2D transform;
            Vector2 size;
            Color color;
        } quad;

        struct
        {
            Vector2 start;
            Vector2 end;
            Color color;
        } line;

    };
};
