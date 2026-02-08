#include "modifier/2d/sprite.h"

#include "resource/texture.h"



void Sprite::init(const mem::Allocator&)
{
    data.texture = nullptr;

    color = Color(255, 255, 255, 255);
    src_rect = Rect2D();
    centered = true;
    flip_h = false;
    flip_v = false;
}

void Sprite::deinit()
{}

void Sprite::render(RenderItemID render_item, const Transform2D& transform, const Rect2D& rect)
{
    if (data.texture == nullptr)
        return;
    
    Rect2D updated_rect = rect;

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
        updated_rect.position = Vector2(rect.size.x / -2.f, rect.size.y / 2.f);
    }

    RenderManager::render_item_draw_sprite(
        render_item, transform, data.texture->texture_id, 
        updated_rect, src_rect, color, static_cast<RenderManager::RenderFlags>(flags)
    );
}

void Sprite::set_texture(Texture2D* new_texture)
{
    data.texture = new_texture;
    if (data.texture == nullptr)
        return;

    src_rect.size = Vector2(new_texture->get_size());
}
