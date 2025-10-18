#include "2d/sprite_2d.h"

#include "graphics/viewport.h"


void Sprite2D::init(const CreateInfo&)
{
    mark(MARK_RENDER);
}

void Sprite2D::render()
{
    if (data.texture == nullptr)
        return;
    
    Vector2 extent = Vector2(data.texture->get_size());
    Rect2D rect = Rect2D(Vector2(), extent);

    u32 flags = 0;
    if (flip_h)
    {
        flags |= Viewport::RENDER_FLAG_FLIP_H;
    }
    if (flip_v)
    {
        flags |= Viewport::RENDER_FLAG_FLIP_V;
    }

    if (centered)
    {
        rect.position = Vector2(rect.size.x / -2.f, rect.size.y / 2.f);
    }

    draw_sprite(
        get_global_transform(), get_texture()->texture_id,
        rect, src_rect, color, flags
    );
}

void Sprite2D::set_texture(Texture2D* new_texture)
{
    data.texture = new_texture;
    if (data.texture == nullptr)
        return;

    src_rect.size = Vector2(new_texture->get_size());
}

