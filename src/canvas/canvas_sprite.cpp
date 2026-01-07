#include "canvas/canvas_sprite.h"

#include "resource/texture.h"


void CanvasSprite::render()
{
    if (data.texture == nullptr)
        return;

    u32 flags = RenderManager::RENDER_FLAG_NONE;
   
    if (flip_h)
    {
        flags |= RenderManager::RENDER_FLAG_FLIP_H;
    }
   
    if (flip_v)
    {
        flags |= RenderManager::RENDER_FLAG_FLIP_V;
    }

    Vector2 size = get_size();
    Rect2D rect = Rect2D(
        Vector2(), size
    );

    draw_canvas_element(
        get_global_transform(), get_texture()->texture_id,
        rect, src_rect, color,
        flags
    );
}

void CanvasSprite::set_texture(Texture2D* new_texture)
{
    data.texture = new_texture;
    if (data.texture == nullptr)
        return;

    src_rect.size = Vector2(new_texture->get_size());
    set_size(src_rect.size);
}

