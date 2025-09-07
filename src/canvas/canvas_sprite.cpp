#include "canvas/canvas_sprite.h"

#include "graphics/graphics.h"
#include "resource/resource_manager.h"


void CanvasSprite::init(const CreateInfo&)
{
    mark(MARK_RENDER);
}

void CanvasSprite::render()
{
    if (data.texture == nullptr)
        return;

    Vector2 texture_extent = Vector2(data.texture->get_size());
    Graphics::draw_canvas_element(
        get_global_transform(),
        texture_extent,
        src_rect, data.texture->texture_id,
        color,
        RenderCommand::BatchFlags(_get_render_flags())
    );
}

void CanvasSprite::set_texture(Texture2D* new_texture)
{
    data.texture = new_texture;
    if (data.texture == nullptr)
        return;

    src_rect.size = Vector2(new_texture->get_size());
}

u32 CanvasSprite::_get_render_flags()
{
    u32 flags = RenderCommand::FLAG_BATCH_NONE;
    flags |= flip_v ? RenderCommand::FLAG_BATCH_FLIP_V : RenderCommand::FLAG_BATCH_NONE;
    flags |= flip_h ? RenderCommand::FLAG_BATCH_FLIP_H : RenderCommand::FLAG_BATCH_NONE;
    return flags;
}

