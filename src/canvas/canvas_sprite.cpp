#include "canvas/canvas_sprite.h"

#include "graphics/viewport.h"
#include "resource/resource_manager.h"


void CanvasSprite::init(const CreateInfo&)
{
    mark(MARK_RENDER);
}

void CanvasSprite::render()
{
    if (data.texture == nullptr)
        return;

    const Vector2 extent = Vector2(data.texture->get_size());
    u32 flags = Viewport::RENDER_FLAG_NONE;
   
    if (flip_h)
    {
        flags |= Viewport::RENDER_FLAG_FLIP_H;
    }
   
    if (flip_v)
    {
        flags |= Viewport::RENDER_FLAG_FLIP_V;
    }

    draw_canvas_element(
        get_global_transform(), get_texture()->texture_id,
        Rect2D(Vector2(), extent), src_rect, color,
        flags
    );
}

void CanvasSprite::set_texture(Texture2D* new_texture)
{
    data.texture = new_texture;
    if (data.texture == nullptr)
        return;

    src_rect.size = Vector2(new_texture->get_size());
}

