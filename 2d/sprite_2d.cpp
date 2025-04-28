#include "2d/sprite_2d.h"

#include "graphics/graphics.h"
#include "io/resource_manager.h"


void Sprite2D::init(const CreateInfo&)
{
    mark_render();
}

void Sprite2D::render()
{
    if(data.texture)
    {
        Vector2 texsize = Vector2(data.texture->get_size());
        Graphics::add_cmd(
            RenderCommand
            {
                .type = RenderCommand::DRAW_SPRITE,
                .sprite =
                {
                    .transform = get_transform(),
                    .texture_extent = texsize,
                    .src_rect = src_rect,
                    .texture = data.texture->texture_id,
                    .color = color,
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

