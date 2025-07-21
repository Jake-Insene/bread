#include "2d/object_2d.h"

#include "input/input.h"
#include "scene/scene_manager.h"
#include "2d/camera_2d.h"


void Object2D::_bind_vtable(VTable&)
{}

void Object2D::init(const CreateInfo&)
{
    Object::data.marks.set(MARK_2D, true);
}

void Object2D::set_position(Vector2 npos)
{
    data.pos_cache = npos;
    data.transform.set_position(npos);
}

Vector2 Object2D::get_position() const
{
    return data.pos_cache;
}

void Object2D::translate(Vector2 t)
{
    data.pos_cache += t;
    data.transform.translate(t);
}

void Object2D::set_scale(Vector2 nscale)
{
    data.scale_cache = nscale;
    data.transform.set_scale(nscale);
}

Vector2 Object2D::get_scale() const
{
    return data.scale_cache;
}

void Object2D::set_rotation(f32 new_rot)
{
    if(data.rot_cache != new_rot)
    {
        data.rot_cache = new_rot;
        data.transform.set_rotation(new_rot);
    }
}

f32 Object2D::get_rotation() const
{
    return data.rot_cache;
}

Transform2D Object2D::get_transform() const
{
    return data.transform;
}

Transform2D Object2D::get_global_transform() const
{
    Object* parent = get_parent();
    if (parent && parent->has_mark(MARK_2D))
    {
        Object2D* p2d = (Object2D*)parent;
        return p2d->get_global_transform() * data.transform;
    }

    return data.transform;
}

Vector2 Object2D::get_local_mouse_position() const
{
    const Camera2D* cam = SceneManager::get_camera_2d();
    const Vector2 display_size = Vector2(SceneManager::get_display_target().get_size());
    const Vector2 screen_pos = Input::get_mouse_position();
    Vector2 local_pos = screen_pos - Vector2(display_size.x, -display_size.y) * 0.5f;

    if (cam)
    {
        local_pos = cam->get_global_transform() * local_pos;
    }

    return local_pos;
}

