#include "physics/area_2d.h"


void Area2D::init(const CreateInfo&)
{
    data.area_id = Physics2D::create_area(this);
    Physics2D::EventOnBodyEnter event_on_body_enter;
    event_on_body_enter.bind(&_on_body_enter);
    Physics2D::area_set_on_body_enter(data.area_id, this, event_on_body_enter);

    Physics2D::EventOnBodyExit event_on_body_exit;
    event_on_body_exit.bind(&_on_body_exit);
    Physics2D::area_set_on_body_exit(data.area_id, this, event_on_body_exit);
}

void Area2D::deinit()
{
    Physics2D::destroy_area(data.area_id);
}

void Area2D::enter()
{}

void Area2D::exit()
{}

void Area2D::add_shape(const Shape2D& new_shape)
{
    Physics2D::area_add_shape(data.area_id, new_shape);
}

void Area2D::remove_shape(usize index)
{
    Physics2D::area_remove_shape(data.area_id, index);
}

usize Area2D::get_shape_count()
{
    return Physics2D::area_get_shape_count(data.area_id);
}

void Area2D::set_shape(usize index, const Shape2D& shape)
{
    Physics2D::area_set_shape(data.area_id, index, shape);
}

Shape2D Area2D::get_shape(usize index)
{
    return Physics2D::area_get_shape(data.area_id, index);
}

void Area2D::set_residence_mask(CollisionMask mask)
{
    data.residence_mask = mask;
    Physics2D::area_set_residence_mask(data.area_id, mask);
}

void Area2D::_on_body_enter(void* _this, Object2D* obj)
{
    Area2D* area = (Area2D*)_this;

    if (area->on_body_enter.has_func() == false)
        return;

    area->on_body_enter.call(obj->cast<Body2D>());
}

void Area2D::_on_body_exit(void* _this, Object2D* obj)
{
    Area2D* area = (Area2D*)_this;

    if (area->on_body_exit.has_func() == false)
        return;

    area->on_body_exit.call(obj->cast<Body2D>());
}
