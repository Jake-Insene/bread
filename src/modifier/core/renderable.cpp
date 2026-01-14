#include "modifier/core/renderable.h"

#include "render/render_manager.h"
#include "resource/texture.h"


void Renderable::draw_sprite(
    RenderItemID render_item, const Transform2D &transform, 
    Texture2D *texture, const Rect2D &rect, const Rect2D &src_rect, 
    const Color &color, u32 flags
)
{
    RenderManager::render_item_draw_sprite(
        render_item, transform, texture->texture_id, 
        rect, src_rect, color, static_cast<RenderManager::RenderFlags>(flags)
    );
}

