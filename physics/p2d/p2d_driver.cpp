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
        .body_set_residence_mask = &P2DDriver::body_set_residence_mask,
        .body_get_residence_mask = &P2DDriver::body_get_residence_mask,
        .body_set_collision_mask = &P2DDriver::body_set_collision_mask,
        .body_get_collision_mask = &P2DDriver::body_get_collision_mask,
    };
}

void P2DDriver::initialize(const mem::Allocator& allocator)
{
    data.allocator = allocator;

    data.gravity = Vector2(0, -98);

    for (usize i = 0; i < Physics2D::MAX_COLLISION_MASKS; i++)
    {
        data.mask_groups[i].active = false;
        data.mask_groups[i].bodies = Array<Physics2D::BodyID>::with_size(data.allocator, 4);
    }

    data.current_bodies = QueueArray<Body, Physics2D::BodyID>::with_size(data.allocator, 4);

    data.collision_callbacks_map = HashMap<CollisionID, CollisionCallback>::with_size(data.allocator, 4);
}

void P2DDriver::shutdown()
{
    for (usize i = 0; i < Physics2D::MAX_COLLISION_MASKS; i++)
    {
        data.mask_groups[i].bodies.destroy();
    }

    data.current_bodies.destroy();
    data.collision_callbacks_map.destroy();
}

void P2DDriver::step(f32 dt)
{
    for (usize i = 0; i < Physics2D::MAX_COLLISION_MASKS; i++)
    {
        CollisionMaskGroup& group = data.mask_groups[i];
        if (!group.active)
        {
            continue;
        }

        for (auto& bodyid : group.bodies)
        {
            _step_body(_get_body(bodyid), dt);
        }
    }

    _resolve_collision_callbacks();
}

Physics2D::BodyID P2DDriver::create_body(Body2D* object)
{
    Physics2D::BodyID id = data.current_bodies.add(Body());
    Body& b = data.current_bodies.get(id);

    b.target = object;
    b.self = id;
    b.residence_mask = Physics2D::CollisionMask(0);
    b.collision_mask = Physics2D::CollisionMask(0);

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
    body_set_residence_mask(body_id, Physics2D::CollisionMask(0));
    body_set_collision_mask(body_id, Physics2D::CollisionMask(0));
    data.current_bodies.remove(body_id);
}

void P2DDriver::body_as_box(Physics2D::BodyID body_id, const Vector2& new_size)
{
    Body& body = _get_body(body_id);
    body.shape.set_size(new_size);
}

void P2DDriver::body_set_type(Physics2D::BodyID body_id, Physics2D::BodyType new_type)
{
    Body& body = _get_body(body_id);
    body.type = new_type;
}

void P2DDriver::body_set_velocity(Physics2D::BodyID body_id, const Vector2& new_velocity)
{
    Body& body = _get_body(body_id);
    body.velocity = new_velocity;
}

Vector2 P2DDriver::body_get_velocity(Physics2D::BodyID body_id)
{
    Body& body = _get_body(body_id);
    return body.velocity;
}

void P2DDriver::body_set_mass(Physics2D::BodyID body_id, f32 new_mass)
{
    Body& body = _get_body(body_id);
    body.mass = new_mass;
}

f32 P2DDriver::body_get_mass(Physics2D::BodyID body_id)
{
    const Body& body = _get_body(body_id);
    return body.mass;
}

void P2DDriver::body_set_friction(Physics2D::BodyID body_id, f32 new_friction)
{
    Body& body = _get_body(body_id);
    body.friction = new_friction;
}

f32 P2DDriver::body_get_friction(Physics2D::BodyID body_id)
{
    const Body& body = _get_body(body_id);
    return body.friction;
}

void P2DDriver::body_apply_force(Physics2D::BodyID body_id, const Vector2& point, const Vector2& force)
{
    Body& body = _get_body(body_id);
}

void P2DDriver::body_apply_impulse(Physics2D::BodyID body_id, const Vector2& point, const Vector2& impulse)
{
    Body& body = _get_body(body_id);
}

void P2DDriver::body_set_fixed_rotation(Physics2D::BodyID body_id, bool enable)
{
    Body& body = _get_body(body_id);
    body.fixed_rotation = enable;
}

bool P2DDriver::body_is_on_floor(Physics2D::BodyID body_id)
{
    const Body& body = _get_body(body_id);
    return body.is_on_floor;
}

void P2DDriver::body_set_residence_mask(Physics2D::BodyID body_id, Physics2D::CollisionMask mask)
{
    Body& body = _get_body(body_id);

    for (usize i = 0; i < Physics2D::MAX_COLLISION_MASKS; i++)
    {
        if ((body.residence_mask & 1 << i) && (mask & 1 << i))
            continue;

        if (mask & 1<<i)
        {
            _mask_group_add(body_id, i);
        }
        else
        {
            _mask_group_remove(body_id, i);
        }
    }

    body.residence_mask = mask;
}

Physics2D::CollisionMask P2DDriver::body_get_residence_mask(Physics2D::BodyID body_id)
{
    return _get_body(body_id).residence_mask;
}

void P2DDriver::body_set_collision_mask(Physics2D::BodyID body_id, Physics2D::CollisionMask mask)
{
    Body& body = _get_body(body_id);
    body.collision_mask = mask;
}

Physics2D::CollisionMask P2DDriver::body_get_collision_mask(Physics2D::BodyID body_id)
{
    return _get_body(body_id).collision_mask;
}

void P2DDriver::_step_body(Body& body, f32 dt)
{
    // Handle collisions
    Vector2 velocity = Vector2(0);
    velocity.x = (data.gravity.x * body.mass + body.velocity.x) * dt;
    velocity.y = (data.gravity.y * body.mass + body.velocity.y) * dt;

    CollisionResult collision_result =
    {
        .advance = Vector2(1),
    };

    body.is_on_floor = false;
    for (usize i = 0; i < Physics2D::MAX_COLLISION_MASKS; i++)
    {
        if (1 << i & body.collision_mask && data.mask_groups[i].active)
        {
            CollisionMaskGroup& group = data.mask_groups[i];
            _check_collision_in_group(group, body, velocity, collision_result);
        }
    }

    body.is_on_floor = collision_result.advance.y && velocity.y < 0 ? false : body.is_on_floor;

    body.target->translate(velocity);

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

void P2DDriver::_check_collision_in_group(CollisionMaskGroup& group, Body& body, 
    Vector2& velocity, CollisionResult& collision_result)
{
    const Vector2 body_position = body.target->get_position();

    for (auto& bodyjd : group.bodies)
    {
        if (body.self == bodyjd || (velocity.x == 0 && velocity.y == 0))
        {
            continue;
        }

        CollisionResult tmp_result =
        {
            .advance = Vector2(1),
        };

        Body& bodyj = _get_body(bodyjd);
        Shape2D test_shape = body.shape;
        Shape2D shapej = bodyj.shape;
        shapej.translate(bodyj.target->get_position());

        test_shape.translate(body_position + Vector2(velocity.x, 0));
        if (test_shape.intersect(shapej))
        {
            // X correction
            tmp_result.advance.x = 0;

            Shape2D real_shape = body.shape;
            real_shape.translate(body_position);

            AABB aabb = real_shape.get_aabb();
            AABB aabbj = shapej.get_aabb();

            // It doesn't make sense to check if velocity.y == 0 here 
            if (velocity.x > 0) // Rightwards collision
            {
                velocity.x = aabbj.min.x - aabb.max.x;
            }
            else if (velocity.x < 0) // Leftwards collision
            {
                velocity.x = aabbj.max.x - aabb.min.x;
            }
        }

        test_shape.translate(Vector2(-velocity.x, velocity.y));
        if (test_shape.intersect(shapej))
        {
            // Y correction
            tmp_result.advance.y = 0;
            body.is_on_floor = velocity.y < 0;

            Shape2D real_shape = body.shape;
            real_shape.translate(body_position);

            AABB aabb = real_shape.get_aabb();
            AABB aabbj = shapej.get_aabb();

            // It doesn't make sense to check if velocity.y == 0 here
            if (tmp_result.advance.x)
            {
                if (velocity.y > 0) // Upwards collision
                {
                    velocity.y = aabbj.max.y - aabb.min.y;
                }
                else if (velocity.y < 0) // Downwards collision
                {
                    velocity.y = aabbj.min.y - aabb.max.y;
                }
            }
        }

        if (tmp_result.advance.x == 0 || tmp_result.advance.y == 0) // Collision in any way
        {
            CollisionID id = CollisionID(body.self, bodyjd);
            if (!data.collision_callbacks_map.has(id) &&
                (body.target->colliding_with.has_func() || bodyj.target->colliding_with.has_func()))
            {
                (void)data.collision_callbacks_map.insert(id,
                    CollisionCallback
                    {
                        .two_ways = body.target->colliding_with.has_func() && bodyj.target->colliding_with.has_func(),
                        .b1 = body.target->colliding_with.has_func() ? body.target : bodyj.target,
                        .b2 = body.target->colliding_with.has_func() ? bodyj.target : body.target,
                    }
                );
            }
        }

        collision_result = tmp_result;
    }
}

void P2DDriver::_resolve_collision_callbacks()
{
    for (auto& itcallback : data.collision_callbacks_map)
    {
        const CollisionCallback& callback = itcallback.second;

        callback.b1->colliding_with.call(callback.b2);
        if (callback.two_ways)
        {
            callback.b2->colliding_with.call(callback.b1);
        }
    }

    data.collision_callbacks_map.clear();
}

void P2DDriver::_mask_group_add(Physics2D::BodyID body_id, usize group_index)
{
    CollisionMaskGroup& group = data.mask_groups[group_index];
    group.active = true;
    (void)group.bodies.add(body_id);
}

void P2DDriver::_mask_group_remove(Physics2D::BodyID body_id, usize group_index)
{
    CollisionMaskGroup& group = data.mask_groups[group_index];
    group.bodies.remove_equal(body_id);
}

