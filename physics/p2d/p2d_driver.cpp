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
        .body_is_on_floor = &P2DDriver::body_is_on_floor,
    };
}

void P2DDriver::initialize(const mem::Allocator& allocator)
{
    data.allocator = allocator;

    data.gravity = Vector2(0, -98);

    data.current_bodies = QueueArray<Body, Physics2D::BodyID>::with_size(data.allocator, 4);
    data.process_bodies = Array<Physics2D::BodyID>::with_size(data.allocator, 60);

    data.collision_callbacks_map = HashMap<CollisionID, u32>::with_size(data.allocator, 4);
    data.collision_callbacks = Array<CollisionCallback>::with_size(data.allocator, 4);
}

void P2DDriver::shutdown()
{
    data.current_bodies.destroy();
    data.process_bodies.destroy();
    data.collision_callbacks_map.destroy();
    data.collision_callbacks.destroy();
}

void P2DDriver::step(f32 dt)
{
    for (auto& bodyid : data.process_bodies)
    {
        Body& body = get_body(bodyid);
        _step_body(body, dt);

        _resolve_collision_callbacks();
    }
}

Physics2D::BodyID P2DDriver::create_body(Body2D* object)
{
    if(data.current_bodies.count == 0)
    {
        data.process_bodies.clear();
    }

    Physics2D::BodyID id = data.current_bodies.add(Body());
    Body& b = data.current_bodies.get(id);
    (void)data.process_bodies.add(id);
    b.index_process = data.process_bodies.count - 1;

    b.target = object;
    b.self = id;

    b.type = (Physics2D::BodyType)object->get_type();
    b.mass = object->get_mass();
    b.friction = object->get_friction();
    b.velocity = object->get_velocity();

    Vector2 position = object->get_position();
    b.shape.set_size(Vector2(1, 1));


    b.last_updated_pos = position;

    return id;
}

void P2DDriver::destroy_body(Physics2D::BodyID body_id)
{
    data.current_bodies.remove(body_id);
    if(data.current_bodies.count == 0)
    {
        data.process_bodies.clear();
    }
}

void P2DDriver::body_as_box(Physics2D::BodyID body_id, const Vector2& new_size)
{
    Body& body = get_body(body_id);
    body.shape.set_size(new_size);
}

void P2DDriver::body_set_type(Physics2D::BodyID body_id, Physics2D::BodyType new_type)
{
    Body& body = get_body(body_id);
    body.type = new_type;
}

void P2DDriver::body_set_velocity(Physics2D::BodyID body_id, const Vector2& new_velocity)
{
    Body& body = get_body(body_id);
    body.velocity = new_velocity;
}

Vector2 P2DDriver::body_get_velocity(Physics2D::BodyID body_id)
{
    Body& body = get_body(body_id);
    return body.velocity;
}

void P2DDriver::body_set_mass(Physics2D::BodyID body_id, f32 new_mass)
{
    Body& body = get_body(body_id);
    body.mass = new_mass;
}

f32 P2DDriver::body_get_mass(Physics2D::BodyID body_id)
{
    const Body& body = get_body(body_id);
    return body.mass;
}

void P2DDriver::body_set_friction(Physics2D::BodyID body_id, f32 new_friction)
{
    Body& body = get_body(body_id);
    body.friction = new_friction;
}

f32 P2DDriver::body_get_friction(Physics2D::BodyID body_id)
{
    const Body& body = get_body(body_id);
    return body.friction;
}

void P2DDriver::body_apply_force(Physics2D::BodyID body_id, const Vector2& point, const Vector2& force)
{
    Body& body = get_body(body_id);
}

void P2DDriver::body_apply_impulse(Physics2D::BodyID body_id, const Vector2& point, const Vector2& impulse)
{
    Body& body = get_body(body_id);
}

void P2DDriver::body_set_fixed_rotation(Physics2D::BodyID body_id, bool enable)
{
    Body& body = get_body(body_id);
    body.fixed_rotation = enable;
}

bool P2DDriver::body_is_on_floor(Physics2D::BodyID body_id)
{
    const Body& body = get_body(body_id);
    return body.is_on_floor;
}

void P2DDriver::_step_body(Body& body, f32 dt)
{
    const Vector2 body_size = body.shape.get_size();
    const Vector2 body_position = body.target->get_position();

    // Handle collisions
    f32 xvelocity = (data.gravity.x * body.mass + body.velocity.x) * dt;
    f32 yvelocity = (data.gravity.y * body.mass + body.velocity.y) * dt;

    bool can_advancex = true;
    bool can_advancey = true;

    for (auto& bodyjd : data.process_bodies)
    {
        if (body.self == bodyjd || (xvelocity == 0 && yvelocity == 0))
        {
            continue;
        }

        Body& bodyj = get_body(bodyjd);
        if (bodyj.target->ignore_collision)
        {
            continue;
        }

        Shape2D test_shape = body.shape;
        Shape2D shapej = bodyj.shape;
        shapej.translate(bodyj.target->get_position());

        test_shape.translate(body_position + Vector2(xvelocity, 0));
        if (test_shape.intersect(shapej))
        {
            // X correction
            can_advancex = false;

            Shape2D real_shape = body.shape;
            real_shape.translate(body_position);

            AABB aabb = real_shape.get_aabb();
            AABB aabbj = shapej.get_aabb();

            // It doesn't make sense to check if xvelocity == 0 here 
            if (xvelocity > 0) // Rightwards collision
            {
                xvelocity = aabbj.min.x - aabb.max.x;
            }
            else if (xvelocity < 0) // Leftwards collision
            {
                xvelocity = aabbj.max.x - aabb.min.x;
            }
        }

        test_shape.translate(Vector2(-xvelocity, yvelocity));
        if (test_shape.intersect(shapej))
        {
            // Y correction
            can_advancey = false;
            body.is_on_floor = yvelocity < 0;

            Shape2D real_shape = body.shape;
            real_shape.translate(body_position);

            AABB aabb = real_shape.get_aabb();
            AABB aabbj = shapej.get_aabb();

            // It doesn't make sense to check if yvelocity == 0 here
            if(can_advancex)
            {
                if (yvelocity > 0) // Upwards collision
                {
                    yvelocity = aabbj.max.y - aabb.min.y;
                }
                else if (yvelocity < 0) // Downwards collision
                {
                    yvelocity = aabbj.min.y - aabb.max.y;
                }
            }
        }

        if (!can_advancex || !can_advancey) // Collision in any way
        {
            CollisionID id = CollisionID(body.self, bodyjd);
            if (!data.collision_callbacks_map.has(id) &&
                (body.target->colliding_with.has_func() || bodyj.target->colliding_with.has_func()))
            {
                (void)data.collision_callbacks.add(
                    CollisionCallback
                    {
                        .two_ways = body.target->colliding_with.has_func() && bodyj.target->colliding_with.has_func(),
                        .b1 = body.target->colliding_with.has_func() ? body.target : bodyj.target,
                        .b2 = body.target->colliding_with.has_func() ? bodyj.target : body.target,
                    }
                    );

                data.collision_callbacks_map.insert(id, data.collision_callbacks.count - 1);
            }
        }
    }

    body.is_on_floor = can_advancey && yvelocity != 0 ? false : body.is_on_floor;

    body.target->translate(Vector2(xvelocity, yvelocity));

    // Debug draw
    Shape2D shape = body.shape;
    shape.translate(body.target->get_position());

    Vector2 top_left = shape.vertices[0];
    Vector2 top_right = shape.vertices[1];
    Vector2 bottom_right = shape.vertices[2];
    Vector2 bottom_left = shape.vertices[3];
    
    Graphics2D::draw_line(Color{ 0, 255, 0, 255 }, top_left, top_right);
    Graphics2D::draw_line(Color{ 0, 255, 0, 255 }, top_right, bottom_right);
    Graphics2D::draw_line(Color{ 0, 255, 0, 255 }, bottom_right, bottom_left);
    Graphics2D::draw_line(Color{ 0, 255, 0, 255 }, bottom_left, top_left);
}

void P2DDriver::_resolve_collision_callbacks()
{
    for (auto& callback : data.collision_callbacks)
    {
        Body2D* b1 = callback.b1;
        Body2D* b2 = callback.b2;

        b1->colliding_with.call(b1, b2);
        if (callback.two_ways)
        {
            b2->colliding_with.call(b2, b1);
        }
    }

    data.collision_callbacks_map.clear();
    data.collision_callbacks.clear();
}
