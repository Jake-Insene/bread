#include "physics/p2d/p2d_driver.h"

#include "graphics/graphics.h"
#include "physics/body_2d.h"



Physics2D::VTable P2DDriver::get_vtable()
{
    return Physics2D::VTable
    {
        .initialize = &P2DDriver::initialize,
        .shutdown = &P2DDriver::shutdown,

        .step = &P2DDriver::step,

        .create_body = &P2DDriver::create_body,
        .destroy_body = &P2DDriver::destroy_body,
        .create_area = &P2DDriver::create_area,
        .destroy_area = &P2DDriver::destroy_area,

        .body_add_shape = &P2DDriver::body_add_shape,
        .body_remove_shape = &P2DDriver::body_remove_shape,
        .body_get_shape_count = &P2DDriver::body_get_shape_count,
        .body_set_shape = &P2DDriver::body_set_shape,
        .body_get_shape = &P2DDriver::body_get_shape,

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
        .body_set_on_collide = &P2DDriver::body_set_on_collide,

        .area_add_shape = &P2DDriver::area_add_shape,
        .area_remove_shape = &P2DDriver::area_remove_shape,
        .area_get_shape_count = &P2DDriver::area_get_shape_count,
        .area_set_shape = &P2DDriver::area_set_shape,
        .area_get_shape = &P2DDriver::area_get_shape,

        .area_set_residence_mask = &P2DDriver::area_set_residence_mask,
        .area_get_residence_mask = &P2DDriver::area_get_residence_mask,

        .area_set_on_body_enter = &P2DDriver::area_set_on_body_enter,
        .area_set_on_body_exit = &P2DDriver::area_set_on_body_exit,
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

    data.active_areas = Array<Physics2D::AreaID>::with_size(data.allocator, 4);

    data.current_bodies = QueueArray<Body, Physics2D::BodyID>::with_size(data.allocator, 4);
    data.current_areas = QueueArray<Area, Physics2D::AreaID>::with_size(data.allocator, 4);

    data.collision_callbacks_map = HashMap<CollisionID, CollisionCallback>::with_size(data.allocator, 4);
}

void P2DDriver::shutdown()
{
    for (usize i = 0; i < Physics2D::MAX_COLLISION_MASKS; i++)
    {
        data.mask_groups[i].bodies.destroy();
    }

    data.active_areas.destroy();

    data.current_bodies.destroy();
    data.current_areas.destroy();
    data.collision_callbacks_map.destroy();
}

void P2DDriver::step(f32 dt)
{
    for (usize i = 0; i < Physics2D::MAX_COLLISION_MASKS; i++)
    {
        CollisionMaskGroup& group = data.mask_groups[i];
        if (group.active == false)
            continue;

        for (auto body_id : group.bodies)
        {
            Body& b = _get_body(body_id);
            _step_body(b, dt);
            _handle_debug_draw_body(b);
        }
    }

    for (usize i = 0; i < Physics2D::MAX_COLLISION_MASKS; i++)
    {
        CollisionMaskGroup& group = data.mask_groups[i];
        if (group.active == false)
            continue;

        for (auto body_id : group.bodies)
        {
            Body& b = _get_body(body_id);
            _check_body_in_areas(b);
        }
        
    }

    _resolve_collision_callbacks();

    for(auto& area_id : data.active_areas)
    {
        Area& area = _get_area(area_id);
        _handle_debug_draw_area(area);
    }

}

Physics2D::BodyID P2DDriver::create_body(Object2D* object)
{
    Physics2D::BodyID id = data.current_bodies.add(Body());
    Body& b = data.current_bodies.get(id);

    b.target = object;
    b.self = id;
    b.residence_mask = Physics2D::CollisionMask(Physics2D::DEFAULT_COLLISION_MASK);
    _mask_group_add(id, 0);

    b.collision_mask = Physics2D::CollisionMask(Physics2D::DEFAULT_COLLISION_MASK);

    b.shapes = Array<Shape2D>::with_size(get_allocator(), 1);
    Vector2 position = object->get_position();
    b.last_updated_pos = position;

    return id;
}

void P2DDriver::destroy_body(Physics2D::BodyID body_id)
{
    Body& body = _get_body(body_id);
    body.shapes.destroy();

    body_set_residence_mask(body_id, Physics2D::CollisionMask(0));
    body_set_collision_mask(body_id, Physics2D::CollisionMask(0));
    data.current_bodies.remove(body_id);
}

Physics2D::AreaID P2DDriver::create_area(Object2D* object)
{
    Physics2D::AreaID id = data.current_areas.add(Area());
    Area& area = data.current_areas.get(id);

    area.target = object;
    area.self = id;
    area.residence_mask = Physics2D::CollisionMask(Physics2D::DEFAULT_COLLISION_MASK);
    _active_area(id);

    area.shapes = Array<Shape2D>::with_size(get_allocator(), 1);
    area.bodies_inside = HashMap<Physics2D::BodyID, Area::BodyInArea>::with_size(get_allocator(), 4);
    return id;
}

void P2DDriver::destroy_area(Physics2D::AreaID area_id)
{
    Area& area = _get_area(area_id);
    area.shapes.destroy();
    area.bodies_inside.destroy();
    _disable_area(area_id);

    data.current_areas.remove(area_id);
}

void P2DDriver::body_add_shape(Physics2D::BodyID body_id, const Shape2D& new_shape)
{
    Body& body = _get_body(body_id);
    (void)body.shapes.add(new_shape);
}

void P2DDriver::body_remove_shape(Physics2D::BodyID body_id, usize index)
{
    Body& body = _get_body(body_id);
    body.shapes.remove(index);
}

usize P2DDriver::body_get_shape_count(Physics2D::BodyID body_id)
{
    Body& body = _get_body(body_id);
    return body.shapes.count;
}

Shape2D P2DDriver::body_get_shape(Physics2D::BodyID body_id, usize index)
{
    Body& body = _get_body(body_id);
    return body.shapes[index];
}

void P2DDriver::body_set_shape(Physics2D::BodyID body_id, usize index, const Shape2D& shape)
{
    Body& body = _get_body(body_id);
    body.shapes[index] = shape;
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

void P2DDriver::body_apply_force(Physics2D::BodyID, const Vector2&, const Vector2&)
{
}

void P2DDriver::body_apply_impulse(Physics2D::BodyID, const Vector2&, const Vector2&)
{
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

void P2DDriver::body_set_on_collide(Physics2D::BodyID body_id, void* _this, Physics2D::EventOnCollide on_collide)
{
    Body& body = _get_body(body_id);
    body._this = _this;
    body.on_collide = on_collide;
}

void P2DDriver::area_add_shape(Physics2D::AreaID area_id, const Shape2D& new_shape)
{
    Area& area = _get_area(area_id);
    (void)area.shapes.add(new_shape);
}

void P2DDriver::area_remove_shape(Physics2D::AreaID area_id, usize index)
{
    Area& area = _get_area(area_id);
    area.shapes.remove(index);
}

usize P2DDriver::area_get_shape_count(Physics2D::AreaID area_id)
{
    Area& area = _get_area(area_id);
    return area.shapes.count;
}

Shape2D P2DDriver::area_get_shape(Physics2D::AreaID area_id, usize index)
{
    Area& area = _get_area(area_id);
    return area.shapes[index];
}

void P2DDriver::area_set_shape(Physics2D::AreaID area_id, usize index, const Shape2D& shape)
{
    Area& area = _get_area(area_id);
    area.shapes[index] = shape;
}

void P2DDriver::area_set_residence_mask(Physics2D::AreaID area_id, Physics2D::CollisionMask mask)
{
    Area& area = _get_area(area_id);
    area.residence_mask = mask;

    if (area.residence_mask == 0)
    {
        _disable_area(area_id);
    }
    else
    {
        _active_area(area_id);
    }
}

Physics2D::CollisionMask P2DDriver::area_get_residence_mask(Physics2D::AreaID area_id)
{
    Area& area = _get_area(area_id);
    return area.residence_mask;
}

void P2DDriver::area_set_on_body_enter(Physics2D::AreaID area_id, void* _this, Physics2D::EventOnBodyEnter on_body_enter)
{
    Area& area = _get_area(area_id);
    area._this = _this;
    area.on_body_enter = on_body_enter;
}

void P2DDriver::area_set_on_body_exit(Physics2D::AreaID area_id, void* _this, Physics2D::EventOnBodyExit on_body_exit)
{
    Area& area = _get_area(area_id);
    area._this = _this;
    area.on_body_exit = on_body_exit;
}

void P2DDriver::_handle_debug_draw_body(Body& body)
{
#if defined(ENABLE_DEBUG_OPTIONS)
    if (Physics2D::get_property("/debug_draw").get<bool>() == false)
        return;

    for (auto& shape : body.shapes)
    {
        Shape2D copy = shape;
        Transform2D transform = body.target->get_global_transform();
        Vector2 center = copy.get_center();
        transform.translate(center);
        Graphics2D::draw_quad(Color{ 0, 255, 0, 127 }, shape.get_size(), transform);
    }
#endif
}

void P2DDriver::_handle_debug_draw_area(Area& area)
{
#if defined(ENABLE_DEBUG_OPTIONS)
    if (Physics2D::get_property("/debug_draw").get<bool>() == false)
        return;

    for (auto& shape : area.shapes)
    {
        Shape2D copy = shape;
        Transform2D transform = area.target->get_global_transform();
        Vector2 center = copy.get_center();
        transform.translate(center);
        Graphics2D::draw_quad(Color{ 0, 255, 0, 127 }, shape.get_size(), transform);
    }
#endif
}

void P2DDriver::_step_body(Body& body, f32 dt)
{
    if (body.shapes.count == 0)
        return;

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
        if (data.mask_groups[i].active == false)
            continue;

        if ((1 << i & body.collision_mask) == 0)
            continue;

        CollisionMaskGroup& group = data.mask_groups[i];
        _check_collision_in_group(group, body, velocity, collision_result);
    }

    body.is_on_floor = collision_result.advance.y && velocity.y < 0 ? false : body.is_on_floor;

    body.target->translate(collision_result.advance * velocity);
}

void P2DDriver::_check_collision_in_group(CollisionMaskGroup& group, Body& body, 
    Vector2& velocity, CollisionResult& collision_result)
{
    for (auto& bodyjd : group.bodies)
    {
        if (body.self == bodyjd)
        {
            continue;
        }

        CollisionResult tmp_result =
        {
            .advance = Vector2(1),
        };

        Body& bodyj = _get_body(bodyjd);
        for (auto& shape : body.shapes)
        {
            _check_collision_on_body(body, shape, bodyj, velocity, tmp_result);

            collision_result.advance.x = collision_result.advance.x ? tmp_result.advance.x : 0;
            collision_result.advance.y = collision_result.advance.y ? tmp_result.advance.y : 0;
        }
    }
}

void P2DDriver::_check_collision_on_body(Body& body, const Shape2D& body_shape, Body& other_body, 
    Vector2& velocity, CollisionResult& collision_result)
{
    const Vector2 body_position = body.target->get_global_transform().get_position();
    const Vector2 bodyj_position = other_body.target->get_global_transform().get_position();

	// We don't want to modify the original shape, so we copy it.
    for (auto other_shape : other_body.shapes)
    {
        CollisionResult tmp_result =
        {
            .advance = Vector2(1),
        };

        Shape2D test_shape = body_shape;
        other_shape.translate(bodyj_position);

        test_shape.translate(body_position + Vector2(velocity.x, 0));
        if (test_shape.intersect(other_shape))
        {
            // X correction
            tmp_result.advance.x = 0;

            Shape2D real_shape = body_shape;
            real_shape.translate(body_position);

            AABB aabb = real_shape.get_aabb();
            AABB aabbj = other_shape.get_aabb();

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
        if (test_shape.intersect(other_shape))
        {
            // Y correction
            tmp_result.advance.y = 0;
            body.is_on_floor = velocity.y < 0;

            Shape2D real_shape = body_shape;
            real_shape.translate(body_position);

            AABB aabb = real_shape.get_aabb();
            AABB aabbj = other_shape.get_aabb();

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

        if ((tmp_result.advance.x == 0 || tmp_result.advance.y == 0)
            && body.on_collide.has_func())
        {
            data.collision_callbacks_map.insert(
                CollisionID(body.self, other_body.self),
                CollisionCallback
                {
                    .body = body.self,
                    .collided = other_body.target,
                }
            );
        }

        collision_result.advance.x = collision_result.advance.x ? tmp_result.advance.x : 0;
        collision_result.advance.y = collision_result.advance.y ? tmp_result.advance.y : 0;
    }
}

void P2DDriver::_check_body_in_areas(Body& body)
{
    for (usize i = 0; i < data.active_areas.count; i++)
    {
        Physics2D::AreaID area_id = data.active_areas[i];
        Area& area = _get_area(area_id);

        if ((body.residence_mask & area.residence_mask) == 0)
            continue;

        const Vector2 area_position = area.target->get_global_transform().get_position();
        _check_body_in_area(area_position, area, body);
    }
}

void P2DDriver::_check_body_in_area(const Vector2& area_position, Area& area, Body& body)
{
    if (area.shapes.count == 0)
        return;

    if (area.on_body_enter.has_func() == false)
        return;

    if (area.on_body_exit.has_func() == false)
        return;
    
    const Vector2 body_position = body.target->get_global_transform().get_position();

    for (auto area_shape : area.shapes)
    {
        // area_shape is not a reference, modify it is safe.
        area_shape.translate(area_position);
        _check_body_in_shape(area_shape, area, body, body_position);
    }
}

void P2DDriver::_check_body_in_shape(const Shape2D& area_shape, Area& area, Body& body, const Vector2& body_position)
{
    if (body.shapes.count == 0)
        return;

    for (auto body_shape : body.shapes)
    {
        body_shape.translate(body_position);

        bool intersect = area_shape.intersect(body_shape);
        if (intersect)
        {
            area.bodies_inside.insert(
                body.self,
                Area::BodyInArea
                {
                    .is_inside = true
                }
            );

            area.on_body_enter.call(area._this, body.target);
        }
        else
        {
            if (area.bodies_inside.has(body.self) == false)
                continue;
         
            area.bodies_inside.get(body.self).is_inside = false;
            area.bodies_inside.remove(body.self);
            area.on_body_exit.call(area._this, body.target);
        }
    }
}

void P2DDriver::_resolve_collision_callbacks()
{
    for (auto& it : data.collision_callbacks_map)
    {
        Body& body = _get_body(it.second.body);
        body.on_collide.call(body._this, it.second.collided);
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

    if(group.bodies.count == 0)
        group.active = false;
}

void P2DDriver::_active_area(Physics2D::AreaID area_id)
{
    Area& area = _get_area(area_id);
    
    if (area.is_active)
        return;

    area.is_active = true;
    (void)data.active_areas.add(area_id);
}

void P2DDriver::_disable_area(Physics2D::AreaID area_id)
{
	Area& area = _get_area(area_id);
    if (area.is_active == false)
        return;

    area.is_active = false;
    data.active_areas.remove_equal(area_id);
}

