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

    struct BindSource
    {
        ResourceID source_id;
    };

    struct DrawSprite
    {
        Transform2D transform;
        // The size of the texture binded.
        Vector2 texture_extent;
        // The size of the rectangle where the texture will be draw.
        Vector2 dest_extent;
        // In texture.
        Rect2D src_rect;
        ResourceID texture;
        Color color;
        SpriteFlags flags;
    };

    struct ClearRT
    {
        ResourceID rid;
        Color color;
        // TODO: add depth
    };

    struct DrawQuad
    {
        Transform2D transform;
        Vector2 size;
        Color color;
    };

    struct DrawLine
    {
        Vector2 start;
        Vector2 end;
        Color color;
    };

    enum CommandType
    {
        NONE = 0,
        BIND_RENDER_TARGET,
        CLEAR_RENDER_TARGET,
        
        DRAW_SPRITE,
        DRAW_QUAD,
        DRAW_LINE,

        SET_SCENE_TRANSFORM,
    } type;
    
    union
    {
        BindSource bind;
        DrawSprite sprite;
        ClearRT clear;
        DrawQuad quad;
        DrawLine line;
        Transform2D transform;
    };
};
