#include "2d/sprite_2d.h"

#include "graphics/graphics.h"
#include "resource/resource_manager.h"


void Sprite2D::enter()
{
    mark(MARK_RENDER);
}

void Sprite2D::render()
{
    if (data.texture == nullptr)
        return;
    
    Vector2 texture_extent = Vector2(data.texture->get_size());
    Graphics::draw_texture(
        get_global_transform(),
        texture_extent,
        src_rect, data.texture->texture_id,
        color,
        RenderCommand::BatchFlags(_get_render_flags())
    );
}

void Sprite2D::set_texture(Texture2D* new_texture)
{
    data.texture = new_texture;
    if (data.texture == nullptr)
        return;

    src_rect.size = Vector2(new_texture->get_size());
}

u32 Sprite2D::_get_render_flags()
{
    u32 flags = RenderCommand::FLAG_BATCH_NONE;
    flags |= centered ? RenderCommand::FLAG_BATCH_NONE : RenderCommand::FLAG_BATCH_TOP_LEFT;
    flags |= flip_v ? RenderCommand::FLAG_BATCH_FLIP_V : RenderCommand::FLAG_BATCH_NONE;
    flags |= flip_h ? RenderCommand::FLAG_BATCH_FLIP_H : RenderCommand::FLAG_BATCH_NONE;
    return flags;
}

