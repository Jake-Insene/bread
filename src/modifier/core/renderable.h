#pragma once
#include "modifier/modifier.h"
#include "render/render_manager.h"


struct Texture2D;


struct Renderable : Modifier
{
    static void draw_sprite(
        RenderItemID render_item, const Transform2D& transform, 
        Texture2D* texture, const Rect2D& rect, const Rect2D& src_rect,
        const Color& color, u32 flags
    );
};
