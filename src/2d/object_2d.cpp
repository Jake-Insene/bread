#include "2d/object_2d.h"

#include "graphics/render_manager.h"
#include "resource/material.h"



void Object2D::_bind_vtable(Object2D::VTable& vtable)
{
    BindVTable(vtable, transform_changed, &Object2D::transform_changed);
}

void Object2D::set_material(Material* new_material)
{
    data.material = new_material;
    RenderManager::item_set_material(get_render_item(), new_material->material_id);
}

Material* Object2D::get_material()
{
    return data.material;
}

void Object2D::init(const CreateInfo&)
{
    mark(MARK_2D);
    data.render_item = RenderManager::create_item();
}

void Object2D::deinit()
{
    RenderManager::destroy_item(data.render_item);
}

void Object2D::enter()
{
    Object* parent = get_parent();
    if (parent)
    {
        Object2D* parent_2d = cast<Object2D>(get_parent());
        if (parent_2d)
        {
            RenderManager::item_set_parent(
                get_render_item(), parent_2d->get_render_item()
            );
        }
    }

    // The parent may not be at the center at this moment so we need to compute the transform as fast as posible to
    // render the object at the correct position or sync the physics engine with the object's transform.
    _update_transform();
}

void Object2D::exit()
{}

void Object2D::transform_changed()
{}

void Object2D::set_position(const Vector2& new_pos)
{
    data.pos_cache = new_pos;
    data.transform.set_position(new_pos);
    _update_transform();
}

Vector2 Object2D::get_position() const
{
    return data.pos_cache;
}

void Object2D::translate(const Vector2& translation)
{
    data.pos_cache += translation;
    data.transform.translate(translation);
    _update_transform();
}

void Object2D::set_scale(const Vector2& new_scale)
{
    if (data.scale_cache == new_scale)
        return;

    data.scale_cache = new_scale;
    data.transform.set_scale(new_scale);
    _update_transform();
}

Vector2 Object2D::get_scale() const
{
    return data.scale_cache;
}

void Object2D::set_rotation(const f32 new_rot)
{
    if (data.rot_cache == new_rot)
        return;
 
    data.rot_cache = new_rot;
    data.transform.set_rotation(new_rot);
    _update_transform();
}

f32 Object2D::get_rotation() const
{
    return data.rot_cache;
}

void Object2D::rotate(const f32 rads)
{
    data.rot_cache += rads;
    data.transform.rotate(rads);
    _update_transform();
}


Transform2D Object2D::get_transform() const
{
    return data.transform;
}

Transform2D Object2D::get_global_transform() const
{
    return data.global_transform_cache;
}

void Object2D::draw_sprite(const Transform2D& transform, TextureID texture, const Rect2D& rect, 
    const Rect2D& src_rect, Color mod_color, u32 flags)
{
    RenderManager::render_item_draw_sprite(
        get_render_item(), transform, texture, rect, src_rect, 
        mod_color, RenderManager::RenderFlags(flags)
    );
}

void Object2D::_update_transform()
{
    data.global_transform_cache = _make_global_transform();
    for (usize i = 0; i < get_child_count(); i++)
    {
        if (Object2D* child = Object::cast<Object2D>(get_child(i)))
        {
            child->_update_transform();
        }
    }

    ObjectCall(transform_changed);
}

Transform2D Object2D::_make_global_transform() const
{
    Object* parent = get_parent();
    if (parent && parent->has_mark(MARK_2D))
    {
        Object2D* p2d = (Object2D*)parent;
        return p2d->get_global_transform() * data.transform;
    }

    return data.transform;
}
