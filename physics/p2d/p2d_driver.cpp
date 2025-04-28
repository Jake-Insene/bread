#include "physics/p2d/p2d_driver.h"

#include "engine/engine.h"
#include "graphics/graphics.h"
#include "physics/body_2d.h"


Vector2 centered_to_top_left(const Vector2& size, const Vector2& center)
{
    return Vector2(center.x - (size.x / 2), center.y + (size.y / 2));
}

Vector2 top_left_to_centered(const Vector2& size, const Vector2& top_left)
{
    return Vector2(top_left.x + (size.x / 2), top_left.y - (size.y / 2));
}

Physics2D::VTable P2DDriver::get_vtable()
{
    return Physics2D::VTable
    {
        .initialize = &P2DDriver::initialize,
        .shutdown = &P2DDriver::shutdown,

        .step = &P2DDriver::step,

        .create_body = &P2DDriver::create_body,
        .destroy_body = &P2DDriver::destroy_body,

        .body_as_box = &P2DDriver::body_as_box,
        .body_set_type = &P2DDriver::body_set_type,
        .body_set_velocity = &P2DDriver::body_set_velocity,
        .body_get_velocity = &P2DDriver::body_get_velocity,
        .body_set_mass = &P2DDriver::body_set_mass,
        .body_get_mass = &P2DDriver::body_get_mass,
        .body_set_friction = &P2DDriver::body_set_friction,
        .body_get_friction = &P2DDriver::body_get_friction,
        .body_apply_force = &P2DDriver::body_apply_force,
        .body_apply_impulse = &P2DDriver::body_apply_impulse,
        .body_set_fixed_rotation = &P2DDriver::body_set_fixed_rotation,
    };
}

void P2DDriver::initialize(mem::Allocator& allocator)
{
    data.allocator = allocator;

    data.gravity = Vector2(0, -980);

    data.current_bodies = QueueArray<Body, Physics2D::BodyID>::with_size(data.allocator, 4);
    data.process_bodies = Array<Physics2D::BodyID>::with_size(data.allocator, 60);
}

void P2DDriver::shutdown()
{
    data.current_bodies.destroy();
    data.process_bodies.destroy();
}

void P2DDriver::step(f32 dt)
{
    for (auto& bodyid : data.process_bodies)
    {
        Body& body = get_body(bodyid);
        Body2D* object = body.target;

        Vector2 position = object->get_position();
        const Vector2 body_size = body.shape.get_size();
        if (position != body.last_updated_pos)
        {
            body.shape.set_position(centered_to_top_left(body_size, position));
        }

        // Debug draw
        Vector2 point1 = body.shape.get_position();
        Vector2 point2 = Vector2(point1.x + body_size.x, point1.y);
        Vector2 point3 = Vector2(point1.x + body_size.x, point1.y - body_size.y);
        Vector2 point4 = Vector2(point1.x, point1.y - body_size.y);
        Graphics2D::draw_line(Color{ 0, 255, 0, 255 }, point1, point2);
        Graphics2D::draw_line(Color{ 0, 255, 0, 255 }, point2, point3);
        Graphics2D::draw_line(Color{ 0, 255, 0, 255 }, point3, point4);
        Graphics2D::draw_line(Color{ 0, 255, 0, 255 }, point4, point1);

        // Handle collisions
        f32 xvelocity = (data.gravity.x * body.mass + body.velocity.x) * dt;
        f32 yvelocity = (data.gravity.y * body.mass + body.velocity.y) * dt;
        Vector2 position_tl = centered_to_top_left(body_size, position);
        bool can_advancex = true;
        bool can_advancey = true;

        for (auto& bodyjd : data.process_bodies)
        {
            if (bodyid == bodyjd)
            {
                continue;
            }

            Body& bodyj = get_body(bodyjd);
            
            Shape2D test_shape = body.shape;
            test_shape.translate(Vector2(xvelocity, 0));
            if (test_shape.intersect(bodyj.shape))
            {
                can_advancex = false;
            }

            test_shape = body.shape;
            test_shape.translate(Vector2(0, yvelocity));
            if (test_shape.intersect(bodyj.shape))
            {
                can_advancey = false;
            }
        }

        body.shape.translate(Vector2(can_advancex * xvelocity, can_advancey * yvelocity));

        Vector2 centered = top_left_to_centered(body_size, body.shape.get_position());
        if (centered != object->get_position())
        {
            object->set_position(centered);
            body.last_updated_pos = centered;
        }
    }
}

Physics2D::BodyID P2DDriver::create_body(Body2D* object_body)
{
    Physics2D::BodyID id = data.current_bodies.add(Body());
    (void)data.process_bodies.add(id);

    Body& b = data.current_bodies.get(id);
    b.target = object_body;
    b.self = id;

    b.type = (Physics2D::BodyType)object_body->get_type();
    b.mass = object_body->get_mass();
    b.friction = object_body->get_friction();
    b.velocity = object_body->get_velocity();

    Vector2 position = object_body->get_position();
    b.shape.set_position(centered_to_top_left(Vector2(1, 1), position));
    b.shape.set_size(Vector2(1, 1));


    b.last_updated_pos = position;

    return id;
}

void P2DDriver::destroy_body(Physics2D::BodyID bodyid)
{
    Body& body = get_body(bodyid);
    data.current_bodies.remove(bodyid);
}

void P2DDriver::body_as_box(Physics2D::BodyID bodyid, const Vector2& new_size)
{
    Body& body = get_body(bodyid);
    body.shape.set_size(new_size*2);
}

void P2DDriver::body_set_type(Physics2D::BodyID bodyid, Physics2D::BodyType new_type)
{
    Body& body = get_body(bodyid);
    body.type = new_type;
}

void P2DDriver::body_set_velocity(Physics2D::BodyID bodyid, const Vector2& new_velocity)
{
    Body& body = get_body(bodyid);
    body.velocity = new_velocity;
}

Vector2 P2DDriver::body_get_velocity(Physics2D::BodyID bodyid)
{
    Body& body = get_body(bodyid);
    return body.velocity;
}

void P2DDriver::body_set_mass(Physics2D::BodyID bodyid, f32 new_mass)
{
    Body& body = get_body(bodyid);
    body.mass = new_mass;
}

f32 P2DDriver::body_get_mass(Physics2D::BodyID bodyid)
{
    const Body& body = get_body(bodyid);
    return body.mass;
}

void P2DDriver::body_set_friction(Physics2D::BodyID bodyid, f32 new_friction)
{
    Body& body = get_body(bodyid);
    body.friction = new_friction;
}

f32 P2DDriver::body_get_friction(Physics2D::BodyID bodyid)
{
    const Body& body = get_body(bodyid);
    return body.friction;
}

void P2DDriver::body_apply_force(Physics2D::BodyID bodyid, const Vector2& point, const Vector2& force)
{
    Body& body = get_body(bodyid);
}

void P2DDriver::body_apply_impulse(Physics2D::BodyID bodyid, const Vector2& point, const Vector2& impulse)
{
    Body& body = get_body(bodyid);
}

void P2DDriver::body_set_fixed_rotation(Physics2D::BodyID bodyid, bool enable)
{
    Body& body = get_body(bodyid);
    body.flags.fixed_rotation = enable;
}
