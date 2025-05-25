#include "2d/sprite_2d.h"

#include "graphics/graphics.h"
#include "io/resource_manager.h"


void Sprite2D::init(const CreateInfo&)
{
    mark(MARK_RENDER);
}

void Sprite2D::render()
{
    if(data.texture)
    {
        Vector2 texture_extent = Vector2(data.texture->get_size());
        Graphics::add_cmd(
            RenderCommand
            {
                .type = RenderCommand::DRAW_SPRITE,
                .sprite =
                {
                    .transform = get_global_transform(),
                    .texture_extent = texture_extent,
                    .dest_extent = texture_extent,
                    .src_rect = src_rect,
                    .texture = data.texture->texture_id,
                    .color = color,
                    .flags = (RenderCommand::SpriteFlags)_get_render_flags(),
                },
            }
        );
    }
}

void Sprite2D::set_texture(Texture2D* new_texture)
{
    data.texture = new_texture;

    src_rect.size = Vector2(new_texture->get_size());
}

u32 Sprite2D::_get_render_flags()
{
    u32 flags = RenderCommand::FLAG_NONE;
    flags |= centered ? RenderCommand::FLAG_NONE : RenderCommand::FLAG_TOP_LEFT;
    flags |= flip_v ? RenderCommand::FLAG_FLIP_V : RenderCommand::FLAG_NONE;
    flags |= flip_h ? RenderCommand::FLAG_FLIP_H : RenderCommand::FLAG_NONE;
    return flags;
}

