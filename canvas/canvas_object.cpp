#include "canvas/canvas_object.h"

#include "scene/scene_manager.h"


void CanvasObject::_bind_vtable(CanvasObject::VTable& vtable)
{
    vtable.is_inside.bind(&CanvasObject::is_inside);
    vtable.get_rect.bind(&CanvasObject::get_rect);
};


void CanvasObject::init(const CreateInfo&)
{
    mark(MARK_RENDER);
    mark(MARK_HANDLE_EVENT);
}

void CanvasObject::enter()
{
    if (get_parent() && !get_parent()->has_mark(MARK_CANVAS))
    {
        SceneManager::_add_root_canvas(this);
    }
}

void CanvasObject::set_position(Vector2 new_pos)
{
    data.transform.set_position(new_pos);
    data.pos_cache = new_pos;
}

Vector2 CanvasObject::get_position() const
{
    return data.pos_cache;
}

void CanvasObject::set_size(Vector2 new_size)
{
    data.size_cache = new_size;
}

Vector2 CanvasObject::get_size() const
{
    return data.size_cache;
}

void CanvasObject::set_scale(Vector2 new_scale)
{
    data.transform.set_scale(new_scale);
    data.scale_cache = new_scale;
}

Vector2 CanvasObject::get_scale() const
{
    return data.scale_cache;
}

void CanvasObject::set_rotation(f32 new_rot)
{
    data.transform.set_rotation(new_rot);
    data.rot_cache = new_rot;
}

f32 CanvasObject::get_rotation() const
{
    return data.rot_cache;
}

Transform2D CanvasObject::get_transform() const
{
    return data.transform;
}

Transform2D CanvasObject::get_global_transform() const
{
    Object* parent = get_parent();
    if (parent && parent->has_mark(MARK_CANVAS))
    {
        CanvasObject* p_canvas = (CanvasObject*)parent;
        return p_canvas->get_global_transform() * data.transform;
    }

    return data.transform;
}


bool CanvasObject::is_inside(const Vector2&) const
{
    return false;
}


Rect2D CanvasObject::get_rect() const
{
    return Rect2D();
}
