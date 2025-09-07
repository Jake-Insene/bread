#pragma once
#include "core/header.h"
#include "resource/resource_id.h"
#include "math/mat4.h"
#include "math/transform_2d.h"
#include "math/rect_2d.h"
#include "math/color.h"

struct RenderCommand
{
    enum BatchFlags
    {
        FLAG_BATCH_NONE = 0x0,
        FLAG_BATCH_TOP_LEFT = 0x1,
        FLAG_BATCH_FLIP_V = 0x2,
        FLAG_BATCH_FLIP_H = 0x4,
        FLAG_BATCH_FONT_CHAR = 0x8,
    };

    struct Sprite
    {
        Transform2D transform;
        // The size of the rectangle where the texture will be draw.
        Vector2 dest_extent;
        // In texture.
        Rect2D src_rect;
        ResourceID texture;
        Color color;
        BatchFlags flags;
    };

    struct CanvasElement
    {
        Transform2D transform;
        // The size of the rectangle where the texture will be draw.
        Vector2 dest_extent;
        // In texture.
        Rect2D src_rect;
        ResourceID texture;
        Color color;
        BatchFlags flags;
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

    struct DrawCircle
    {
        Vector2 point;
        Color color;
        f32 radius;
    };

    enum CommandType
    {
        NONE = 0,
        DRAW_SPRITE,
        DRAW_CANVAS_ELEMENT,
        DRAW_QUAD,
        DRAW_LINE,
        DRAW_CIRCLE,

        SET_SCENE_TRANSFORM,
    } type;
    
    union
    {
        Sprite sprite;
        CanvasElement canvas_element;
        DrawQuad quad;
        DrawLine line;
        DrawCircle circle;
        Transform2D transform;
    };
};
