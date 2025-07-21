#pragma once
#include "core/header.h"
#include "resource/resource_id.h"
#include "math/mat4.h"
#include "math/transform_2d.h"
#include "math/rect_2d.h"
#include "math/color.h"

struct RenderCommand
{
    enum SpriteFlags
    {
        FLAG_SPRITE_NONE = 0x0,
        FLAG_SPRITE_TOP_LEFT = 0x1,
        FLAG_SPRITE_FLIP_V = 0x2,
        FLAG_SPRITE_FLIP_H = 0x4,
    };

    enum CanvasFlags
    {
        FLAG_CANVAS_NONE = 0x0,
        FLAG_CANVAS_FLIP_V = 0x1,
        FLAG_CANVAS_FLIP_H = 0x2,
        FLAG_CANVAS_FONT_CHAR = 0x4,
    };

    struct BindSource
    {
        ResourceID source_id;
    };

    struct Sprite
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

    struct CanvasElement
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
        CanvasFlags flags;
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
        DRAW_CANVAS_ELEMENT,
        DRAW_QUAD,
        DRAW_LINE,

        SET_SCENE_TRANSFORM,
    } type;
    
    union
    {
        BindSource bind;
        Sprite sprite;
        CanvasElement canvas_element;
        ClearRT clear;
        DrawQuad quad;
        DrawLine line;
        Transform2D transform;
        Mat4 matrix;
    };
};
