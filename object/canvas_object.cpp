#include "object/canvas_object.h"


void CanvasObject::_bind_vtable(CanvasObject::VTable& vtable)
{
    vtable.is_inside.bind(&CanvasObject::is_inside);
};


void CanvasObject::init(const CreateInfo&)
{
    mark(MARK_RENDER);
    
    data.transform = Transform2D();
}

void CanvasObject::set_position(Vector2 npos)
{
    data.transform.set_position(npos);
}

Vector2 CanvasObject::get_position() const
{
    return data.transform.get_position();
}

void CanvasObject::set_scale(Vector2 nscale)
{
    data.transform.set_scale(nscale);
}

Vector2 CanvasObject::get_scale() const
{
    return data.transform.get_scale();
}

bool CanvasObject::is_inside(Vector2) const
{
    return false;
}
