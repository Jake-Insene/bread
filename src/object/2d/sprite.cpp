#include "object/2d/sprite.h"
#include "object/core/renderable.h"
#include "render/render_manager.h"



void Sprite::init(const CreateInfo& info)
{
    Renderable::init(info);
}

void Sprite::deinit()
{
    Renderable::deinit();
}


void Sprite::render(const Transform2D& transform)
{
    if (data.texture == nullptr)
        return;
    
    Vector2 extent = Vector2(data.texture->get_size());
    Rect2D rect = Rect2D(Vector2(), extent);

    u32 flags = 0;
    if (flip_h)
    {
        flags |= RenderManager::RENDER_FLAG_FLIP_H;
    }
    if (flip_v)
    {
        flags |= RenderManager::RENDER_FLAG_FLIP_V;
    }

    if (centered)
    {
        rect.position = Vector2(rect.size.x / -2.f, rect.size.y / 2.f);
    }

    draw_sprite(transform, get_texture(), rect, src_rect, color, flags);
}

void Sprite::set_texture(Texture2D* new_texture)
{
    data.texture = new_texture;
    if (data.texture == nullptr)
        return;

    src_rect.size = Vector2(new_texture->get_size());
}
