#include "object/core/render_object.h"

#include "resource/texture.h"


void RenderObject::init(const CreateInfo& info)
{
    Object::init(info);
    data.render_item = RenderManager::create_item();
}


void RenderObject::deinit()
{
    RenderManager::destroy_item(data.render_item);
    
    Object::deinit();
}


void RenderObject::draw_sprite(const Transform2D& transform, Texture2D* texture, const Rect2D& rect, const Rect2D& src_rect, Color color, u32 flags)
{
    RenderManager::render_item_draw_sprite(
        data.render_item, transform, texture->texture_id, 
        rect, src_rect, color, static_cast<RenderManager::RenderFlags>(flags)
    );
}

