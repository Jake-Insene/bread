#include "2d/object_2d.h"

void Object2D::_bind_vtable(VTable&)
{}


void Object2D::init(const CreateInfo&)
{
    Object::data.flags.set(FLAG_2D, true);
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

void Object2D::set_rotation(f32 nrot)
{
    data.rot_cache = nrot;
    data.transform.set_rotation(nrot);
}

f32 Object2D::get_rotation() const
{
    return data.rot_cache;
}

Transform2D Object2D::get_transform() const
{
    Object* parent = get_parent();
    if(parent && parent->has_flag(FLAG_2D))
    {
        Object2D* p2d = (Object2D*)parent;
        return p2d->get_transform() * data.transform;
    }

    return data.transform;
}
