#include "physics/p2d/p2d_driver.h"

#include "2d/object_2d.h"
#include "graphics/viewport.h"
#include "physics/p2d/p2d_types.h"
#include "physics/physics_2d.h"



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

        .body_set_shape = &P2DDriver::body_set_shape,
        .body_get_shape = &P2DDriver::body_get_shape,

        .body_set_type = &P2DDriver::body_set_type,
        .body_set_velocity = &P2DDriver::body_set_velocity,
        .body_get_velocity = &P2DDriver::body_get_velocity,
        .body_set_angular_velocity = &P2DDriver::body_set_angular_velocity,
        .body_get_angular_velocity = &P2DDriver::body_get_angular_velocity,
        .body_set_mass = &P2DDriver::body_set_mass,
        .body_get_mass = &P2DDriver::body_get_mass,
        .body_set_friction = &P2DDriver::body_set_friction,
        .body_get_friction = &P2DDriver::body_get_friction,
        .body_set_air_friction = &P2DDriver::body_set_air_friction,
        .body_get_air_friction = &P2DDriver::body_get_air_friction,
        .body_set_bounce = &P2DDriver::body_set_bounce,
        .body_get_bounce = &P2DDriver::body_get_bounce,
        .body_apply_force = &P2DDriver::body_apply_force,
        .body_apply_impulse = &P2DDriver::body_apply_impulse,
        .body_set_fixed_rotation = &P2DDriver::body_set_fixed_rotation,

        .body_is_on_floor = &P2DDriver::body_is_on_floor,
        .body_is_on_ceil = &P2DDriver::body_is_on_ceil,
        
        .body_set_residence_mask = &P2DDriver::body_set_residence_mask,
        .body_get_residence_mask = &P2DDriver::body_get_residence_mask,
        .body_set_collision_mask = &P2DDriver::body_set_collision_mask,
        .body_get_collision_mask = &P2DDriver::body_get_collision_mask,
        .body_set_on_collide = &P2DDriver::body_set_on_collide,

        .area_set_shape = &P2DDriver::area_set_shape,
        .area_get_shape = &P2DDriver::area_get_shape,

        .area_set_residence_mask = &P2DDriver::area_set_residence_mask,
        .area_get_residence_mask = &P2DDriver::area_get_residence_mask,

        .area_set_on_body_enter = &P2DDriver::area_set_on_body_enter,
        .area_set_on_body_exit = &P2DDriver::area_set_on_body_exit,

        .property_change = &P2DDriver::property_change,
    };
}

void P2DDriver::initialize(const mem::Allocator& allocator)
{
    data.allocator = allocator;

    data.gravity = Physics2D::get_property("/gravity").get<Vector2>();

    for (usize i = 0; i < Physics2D::MAX_COLLISION_MASKS; i++)
    {
        data.mask_groups[i].active = false;
        data.mask_groups[i].bodies = Array<Physics2D::BodyID>::with_size(data.allocator, 4);
    }

    data.active_areas = Array<Physics2D::AreaID>::with_size(data.allocator, 4);
    data.active_bodies = Array<Physics2D::BodyID>::with_size(data.allocator, 4);

    data.current_bodies = FreeList<P2DBody, Physics2D::BodyID>::with_size(data.allocator, 4);
    data.current_areas = FreeList<Area, Physics2D::AreaID>::with_size(data.allocator, 4);

    data.collision_callbacks_map = HashMap<CollisionID, CollisionCallback>::with_size(data.allocator, 4);
    data.resolved_pairs = HashMap<CollisionID, bool>::with_size(data.allocator, 16);

    data.tile_size = Physics2D::get_property("/tile_size").get<i32>();
	data.world_tiles = HashMap<PhysicsTileCoord, PhysicsTile>::with_size(data.allocator, InitialWorldTiles);

    data.fixed_step = Physics2D::get_property("/fixed_step").get<f32>();
    data.accumulator = 0.0f;
}

void P2DDriver::shutdown()
{
    for (usize i = 0; i < Physics2D::MAX_COLLISION_MASKS; i++)
    {
        data.mask_groups[i].bodies.destroy();
    }

    data.active_areas.destroy();
    data.active_bodies.destroy();

    data.current_bodies.destroy();
    data.current_areas.destroy();
    data.collision_callbacks_map.destroy();
    data.resolved_pairs.destroy();

    for (auto& entry : data.world_tiles.entries)
    {
        if(entry)
        {
            entry->kv.second.bodies.destroy();
            entry->kv.second.areas.destroy();
        }
    }
    data.world_tiles.destroy();
}

void P2DDriver::step(f32 dt)
{
    data.accumulator += dt;
    while (data.accumulator >= data.fixed_step)
    {
        _step_fixed(data.fixed_step);
        data.accumulator -= data.fixed_step;
    }

    for (auto body_id : data.active_bodies)
    {
        P2DBody& body = _get_body(body_id);
        _handle_debug_draw_body(body);
    }

    for (auto area_id : data.active_areas)
    {
        Area& area = _get_area(area_id);
        _handle_debug_draw_area(area);
    }

    if (data.debug_draw)
    {
        P2DBody& first_body = _get_body(data.active_bodies[0]);
        Viewport* viewport = first_body.target->get_viewport();
        for (auto& entry : data.world_tiles)
        {
            PhysicsTileCoord coord = entry.second.coord;

            const f32 ts = f32(_get_tile_size());
            Vector2 min = Vector2((coord.x ) * ts, (coord.y) * ts);
            Vector2 max = min + Vector2(ts, ts);
            viewport->render_item_draw_line(first_body.target->get_render_item(), Vector2(min.x, min.y), Vector2(max.x, min.y), Color(128,128,128,255));
            viewport->render_item_draw_line(first_body.target->get_render_item(), Vector2(max.x, min.y), Vector2(max.x, max.y), Color(128,128,128,255));
            viewport->render_item_draw_line(first_body.target->get_render_item(), Vector2(max.x, max.y), Vector2(min.x, max.y), Color(128,128,128,255));
            viewport->render_item_draw_line(first_body.target->get_render_item(), Vector2(min.x, max.y), Vector2(min.x, min.y), Color(128,128,128,255));
        }
    }
}

Physics2D::BodyID P2DDriver::create_body(Object2D* object)
{
    Physics2D::BodyID id = data.current_bodies.add(P2DBody());
    P2DBody& new_body = data.current_bodies.get(id);
    (void)data.active_bodies.add(id);

    new_body.target = object;
    new_body.self = id;
    new_body.type = Physics2D::DYNAMIC;
    new_body.residence_mask = Physics2D::CollisionMask(Physics2D::DEFAULT_COLLISION_MASK);
    _mask_group_add(id, 0);

    new_body.collision_mask = Physics2D::CollisionMask(Physics2D::DEFAULT_COLLISION_MASK);

    new_body.set_shape(P2DShape());
    new_body.set_mass(1.f);
    new_body.set_friction(1.f);
    new_body.set_air_friction(1.f);
    new_body.set_bounce(0.f);
    new_body.compute_inertia();
    new_body.set_velocity(Vector2());
    new_body.set_angular_velocity(0.f);
    new_body.has_pending_static_collision = false;

    new_body.tiles_on = Array<PhysicsTileCoord>::with_size(get_allocator(), 4);

    return id;
}

void P2DDriver::destroy_body(Physics2D::BodyID body_id)
{
    P2DBody& body = _get_body(body_id);
    body_set_residence_mask(body_id, Physics2D::CollisionMask(0));
    body_set_collision_mask(body_id, Physics2D::CollisionMask(0));
    data.active_bodies.remove_equal(body_id);

    // Removeing from a tile
    for (auto& tile_id : body.tiles_on)
    {
        PhysicsTile& tile = _get_or_create_tile(tile_id);
        tile.bodies.remove_equal(body.self);
    }
    body.tiles_on.destroy();

    data.current_bodies.remove(body_id);
}

Physics2D::AreaID P2DDriver::create_area(Object2D* object)
{
    Physics2D::AreaID id = data.current_areas.add(Area());
    Area& new_area = data.current_areas.get(id);

    new_area.target = object;
    new_area.self = id;
    new_area.residence_mask = Physics2D::CollisionMask(Physics2D::DEFAULT_COLLISION_MASK);
    _active_area(id);

    new_area.shape = P2DShape();
    new_area.bodies_inside = HashMap<Physics2D::BodyID, Area::BodyInArea>::with_size(get_allocator(), 4);
    new_area.tiles_on = Array<PhysicsTileCoord>::with_size(get_allocator(), 4);

    return id;
}

void P2DDriver::destroy_area(Physics2D::AreaID area_id)
{
    Area& area = _get_area(area_id);
    area.bodies_inside.destroy();
    _disable_area(area_id);
    area.tiles_on.destroy();

    data.current_areas.remove(area_id);
}

void P2DDriver::body_set_shape(Physics2D::BodyID body_id, const Shape2D& shape)
{
    P2DBody& body = _get_body(body_id);
    body.set_shape(P2DShape::from_shape_2d(shape));
    _body_recompute_tiles(body);
}

Shape2D P2DDriver::body_get_shape(Physics2D::BodyID body_id)
{
    P2DBody& body = _get_body(body_id);
    return body.get_shape().to_shape_2d();
}

void P2DDriver::body_set_type(Physics2D::BodyID body_id, Physics2D::BodyType new_type)
{
    P2DBody& body = _get_body(body_id);
    body.type = new_type;
    switch (new_type)
    {
    case Physics2D::STATIC:
        body.set_mass(0.f);
        body.set_velocity(Vector2());
        break;
    case Physics2D::KINEMATIC:
        body.set_mass(0.f);
        break;
    case Physics2D::DYNAMIC:
        body.set_mass(body.get_mass() > 0.f ? body.get_mass() : 1.f);
        break;
    default:
        break;
    }
}

void P2DDriver::body_set_velocity(Physics2D::BodyID body_id, const Vector2& new_velocity)
{
    P2DBody& body = _get_body(body_id);
    body.set_velocity(new_velocity);
}

Vector2 P2DDriver::body_get_velocity(Physics2D::BodyID body_id)
{
    P2DBody& body = _get_body(body_id);
    return body.get_velocity();
}

void P2DDriver::body_set_angular_velocity(Physics2D::BodyID body_id, f32 angular_velocity)
{
    P2DBody& body = _get_body(body_id);
    body.set_angular_velocity(angular_velocity);
}

f32 P2DDriver::body_get_angular_velocity(Physics2D::BodyID body_id)
{
    const P2DBody& body = _get_body(body_id);
    return body.get_angular_velocity();
}

void P2DDriver::body_set_mass(Physics2D::BodyID body_id, f32 new_mass)
{
    P2DBody& body = _get_body(body_id);
    body.set_mass(new_mass);
}

f32 P2DDriver::body_get_mass(Physics2D::BodyID body_id)
{
    const P2DBody& body = _get_body(body_id);
    return body.get_mass();
}

void P2DDriver::body_set_friction(Physics2D::BodyID body_id, f32 new_friction)
{
    P2DBody& body = _get_body(body_id);
    body.set_friction(new_friction);
}

f32 P2DDriver::body_get_friction(Physics2D::BodyID body_id)
{
    const P2DBody& body = _get_body(body_id);
    return body.get_friction();
}

void P2DDriver::body_set_air_friction(Physics2D::BodyID body_id, f32 new_air_friction)
{
    P2DBody& body = _get_body(body_id);
    body.set_air_friction(new_air_friction);
}

f32 P2DDriver::body_get_air_friction(Physics2D::BodyID body_id)
{
    const P2DBody& body = _get_body(body_id);
    return body.get_air_friction();
}

void P2DDriver::body_set_bounce(Physics2D::BodyID body_id, f32 new_bounce)
{
    P2DBody& body = _get_body(body_id);
    body.set_bounce(new_bounce);
}

f32 P2DDriver::body_get_bounce(Physics2D::BodyID body_id)
{
    const P2DBody& body = _get_body(body_id);
    return body.get_bounce();
}

void P2DDriver::body_apply_force(Physics2D::BodyID body_id, const Vector2&, const Vector2& force)
{
    P2DBody& body = _get_body(body_id);
    body.add_force(force);
}

void P2DDriver::body_apply_impulse(Physics2D::BodyID body_id, const Vector2&, const Vector2& impulse)
{
    P2DBody& body = _get_body(body_id);
    if (body.get_inv_mass() > 0.f)
    {
        body.set_velocity(body.get_velocity() + impulse * body.get_inv_mass());
    }
}

void P2DDriver::body_set_fixed_rotation(Physics2D::BodyID body_id, bool enable)
{
    P2DBody& body = _get_body(body_id);
    body.fixed_rotation = enable;
}

bool P2DDriver::body_is_on_floor(Physics2D::BodyID body_id)
{
    const P2DBody& body = _get_body(body_id);
    return body.is_on_floor;
}

bool P2DDriver::body_is_on_ceil(Physics2D::BodyID body_id)
{
    const P2DBody& body = _get_body(body_id);
    return body.is_on_ceil;
}

void P2DDriver::body_set_residence_mask(Physics2D::BodyID body_id, Physics2D::CollisionMask mask)
{
    P2DBody& body = _get_body(body_id);

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
    P2DBody& body = _get_body(body_id);
    body.collision_mask = mask;
}

Physics2D::CollisionMask P2DDriver::body_get_collision_mask(Physics2D::BodyID body_id)
{
    return _get_body(body_id).collision_mask;
}

void P2DDriver::body_set_on_collide(Physics2D::BodyID body_id, void* _this, Physics2D::EventOnCollide on_collide)
{
    P2DBody& body = _get_body(body_id);
    body._this = _this;
    body.on_collide = on_collide;
}

void P2DDriver::area_set_shape(Physics2D::AreaID area_id, const Shape2D& shape)
{
    Area& area = _get_area(area_id);
    area.shape = P2DShape::from_shape_2d(shape);

    // Getting tiles in area
    _area_recompute_tiles(area);
}

Shape2D P2DDriver::area_get_shape(Physics2D::AreaID area_id)
{
    Area& area = _get_area(area_id);
    return area.shape.to_shape_2d();
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

void P2DDriver::property_change(StringView property_name, PropertyValue new_value)
{
    if (property_name.equals("/tile_size"))
    {

    }
    else if (property_name.equals("/gravity"))
    {
        data.gravity = new_value.get<Vector2>();
    }
    else if (property_name.equals("/debug_draw"))
    {
        data.debug_draw = new_value.get<bool>();
    }
    else if (property_name.equals("/fixed_step"))
    {
        data.fixed_step = new_value.get<f32>();
    }
}

void P2DDriver::_step_fixed(f32 dt)
{
    for (auto area_id : data.active_areas)
    {
        Area& area = _get_area(area_id);
        _area_recompute_tiles(area);
        _check_area_collision(area);
    }

    data.resolved_pairs.clear();

    for(auto body_id : data.active_bodies)
    {
        P2DBody& body = _get_body(body_id);
        _move_body(body, dt);
        _body_recompute_tiles(body);
        _check_body_collision(body);
    }

    // Resolve pending static collisions
    for (auto body_id : data.active_bodies)
    {
        P2DBody& body = _get_body(body_id);
        if (body.has_pending_static_collision)
        {
            P2DBody& other_body = _get_body(body.pending_static_collision.other);
            P2DCollision::positional_correction(body.pending_static_collision.manifold, body, other_body);
            P2DCollision::resolve_collision(body.pending_static_collision.manifold, body, other_body);
        }
     
        body.has_pending_static_collision = false;
        body.pending_static_collision = PendingCollision();
        body.moved = false;
    }

    _resolve_collision_callbacks();
}

void P2DDriver::_handle_debug_draw_body(P2DBody& body)
{
    if (data.debug_draw == false)
        return;

    P2DShape shape = body.get_shape();
    Transform2D transform = body.target->get_global_transform();
    shape.apply_transform(transform);

    for (usize i = 0; i < 4; i++)
    {
        Vector2 point1 = shape.vertices[i];
        Vector2 point2 = shape.vertices[(i + 1) % 4];

        body.target->get_viewport()->render_item_draw_line(
            body.target->get_render_item(),
            point1, point2, Color(255, 0, 0, 255)
        );
    }

    body.target->get_viewport()->render_item_draw_circle(
        body.target->get_render_item(),
        shape.get_centroid(), 1.f, Color(255, 0, 0, 255)
    );

    body.target->get_viewport()->render_item_draw_circle(
        body.target->get_render_item(),
        shape.aabb.min, 1.f, Color(0, 255, 0, 255)
    );

    body.target->get_viewport()->render_item_draw_circle(
        body.target->get_render_item(),
        shape.aabb.max, 1.f, Color(0, 0, 255, 255)
    );
}

void P2DDriver::_handle_debug_draw_area(Area& area)
{
    if (data.debug_draw == false)
        return;

    P2DShape shape = area.shape;
    Transform2D transform = area.target->get_global_transform();
    shape.apply_transform(transform);

    for (usize i = 0; i < 4; i++)
    {
        Vector2 point1 = shape.vertices[i];
        Vector2 point2 = shape.vertices[(i + 1) % 4];

        area.target->get_viewport()->render_item_draw_line(
            area.target->get_render_item(),
            point1, point2, Color(0, 0, 255, 255)
        );
    }

    area.target->get_viewport()->render_item_draw_circle(
        area.target->get_render_item(),
        shape.get_centroid(), 1.f, Color(255, 0, 0, 255)
    );
}

void P2DDriver::_move_body(P2DBody& body, f32 dt)
{
    if (body.type == Physics2D::STATIC)
        return;

    if (body.moved)
        return;

    body.moved = true;

    body.add_force(data.gravity * body.get_mass());
    body.step(dt);

    body.is_on_floor = false;
    body.is_on_ceil = false;
    body.has_pending_static_collision = false;
}

void P2DDriver::_check_body_collision(P2DBody& body)
{
    for (auto tile_coord : body.tiles_on)
    {
        PhysicsTile& tile = _get_or_create_tile(tile_coord);
        _check_body_collisions_on_tile(body, tile);
    }
}

void P2DDriver::_check_body_collisions_on_tile(P2DBody& body, PhysicsTile& tile)
{
	P2DShape body_shape = body.get_shape();
	Transform2D global_transform = body.target->get_global_transform();
	body_shape.apply_transform(global_transform);

    for (auto other_body_id : tile.bodies)
    {
        if (body.self == other_body_id)
            continue;

        P2DBody& other_body = _get_body(other_body_id);
        // Ignore collision between static objects.
        if(body.type == Physics2D::STATIC && other_body.type == Physics2D::STATIC)
            continue;

        if ((body.collision_mask & other_body.residence_mask) == 0)
            continue;

        P2DShape other_shape = other_body.get_shape();
        other_shape.apply_transform(other_body.target->get_global_transform());

        bool collided = false; 
        CollisionManifold manifold;
        collided = body_shape.aabb.intersecs(other_shape.aabb);
        if (collided)
        {
            manifold = P2DCollision::polygon_v_polygon(body_shape, other_shape);
            collided = manifold.valid;
        }

        if (!collided)
        {
            continue;
        }

        body.is_on_floor = manifold.normal.y < 0;
        body.is_on_ceil = manifold.normal.y > 0;

        other_body.is_on_floor = manifold.normal.y > 0;
        other_body.is_on_ceil = manifold.normal.y < 0;

        if (other_body.type == Physics2D::STATIC)
        {
            if(body.has_pending_static_collision)
            {
                if(body.pending_static_collision.manifold.depth < manifold.depth)
                {
                    body.pending_static_collision = PendingCollision{manifold, other_body.self};
                }
            }
            else
            {
                body.has_pending_static_collision = true;
                body.pending_static_collision = PendingCollision{manifold, other_body.self};
            }
        }
        else
        {
            // Resolve non-static immediately
            CollisionID pair_id = CollisionID{body.self, other_body.self};
            if (!data.resolved_pairs.has(pair_id))
            {
                data.resolved_pairs.insert(pair_id, true);
                P2DCollision::positional_correction(manifold, body, other_body);
                P2DCollision::resolve_collision(manifold, body, other_body);
            }
        }

        if (body.on_collide.has_func())
        {
            data.collision_callbacks_map.insert(
                CollisionID(body.self, other_body.self),
                CollisionCallback
                {
                    .body = body.self,
                    .collided = other_body.self,
                }
            );
        }
    }
}

void P2DDriver::_check_area_collision(Area& area)
{
    for (auto tile_coord : area.tiles_on)
    {
        PhysicsTile& tile = _get_or_create_tile(tile_coord);
        _check_area_collision_on_tile(area, tile);
    }
}

void P2DDriver::_check_area_collision_on_tile(Area& area, PhysicsTile& tile)
{
    P2DShape area_shape = area.shape;
    area_shape.apply_transform(area.target->get_global_transform());

    for(auto& body_id : tile.bodies)
    {
        P2DBody& body = _get_body(body_id);
        if ((area.residence_mask & body.residence_mask) == 0)
            continue;

        P2DShape other_shape = body.get_shape();
        other_shape.apply_transform(body.target->get_global_transform());

        bool collided = false;
        collided = area_shape.aabb.intersecs(other_shape.aabb);
        if(collided)
        {
            CollisionManifold manifold = P2DCollision::polygon_v_polygon(area_shape, other_shape);
            collided = manifold.valid;
        }

        if (collided)
        {
            area.bodies_inside.insert(
                body.self,
                Area::BodyInArea
                {
                    .is_inside = true
                }
            );

            if (area.on_body_enter.has_func() == false)
                continue;

            area.on_body_enter.call(area._this, body.target);
        }
        else
        {
            if (area.bodies_inside.has(body.self) == false)
                continue;

            area.bodies_inside.get(body.self).is_inside = false;
            area.bodies_inside.remove(body.self);

            if (area.on_body_exit.has_func() == false)
                continue;

            area.on_body_exit.call(area._this, body.target);
        }
    }
}

void P2DDriver::_resolve_collision_callbacks()
{
    for (auto& [hash, value] : data.collision_callbacks_map)
    {
        P2DBody& body = _get_body(value.body);
        P2DBody& collided = _get_body(value.collided);
        body.on_collide.call(body._this, collided.target);

        if (collided.on_collide.has_func())
        {
            collided.on_collide.call(collided._this, body.target);
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
    if (!area.is_active)
        return;

    area.is_active = false;
    data.active_areas.remove_equal(area_id);
}

PhysicsTileCoord P2DDriver::_convert_to_world_tile(const Vector2& point)
{
    const Vector2 normalized_point = point / f32(_get_tile_size());
    PhysicsTileCoord tile = {};

    tile.x = math::floor(normalized_point.x);
    tile.y = math::floor(normalized_point.y);

    return tile;
}


void P2DDriver::_body_recompute_tiles(P2DBody& body)
{
    // Removing from old tiles
    for (auto& tile_id : body.tiles_on)
    {
        PhysicsTile& tile = _get_or_create_tile(tile_id);
        tile.bodies.remove_equal(body.self);
    }
    body.tiles_on.clear();

    // Getting tiles in shape
    P2DShape body_shape = body.get_shape();
    Transform2D global_transform = body.target->get_global_transform();
    body_shape.apply_transform(global_transform);

    AABB aabb = body_shape.aabb;
    PhysicsTileCoord min_tile = _convert_to_world_tile(aabb.min);
    PhysicsTileCoord max_tile = _convert_to_world_tile(aabb.max);

    for (i32 x = min_tile.x; x <= max_tile.x; x++)
    {
        for (i32 y = min_tile.y; y <= max_tile.y; y++)
        {
            PhysicsTileCoord tile_coord;
            tile_coord.x = x;
            tile_coord.y = y;
            (void)body.tiles_on.add(tile_coord);

            PhysicsTile& tile = _get_or_create_tile(tile_coord);
            bool can_insert = true;
            for (auto body_id : tile.bodies)
            {
                if (body_id == body.self)
                {
                    can_insert = false;
                    break;
                }
            }

            if (can_insert)
                (void)tile.bodies.add(body.self);
        }
    }
}

void P2DDriver::_area_recompute_tiles(Area& area)
{
    area.tiles_on.clear();

    // Getting tiles in shape
    P2DShape body_shape = area.shape;
    Transform2D global_transform = area.target->get_global_transform();
    body_shape.apply_transform(global_transform);

    auto aabb = body_shape.aabb;
    PhysicsTileCoord min_tile = _convert_to_world_tile(aabb.min);
    PhysicsTileCoord max_tile = _convert_to_world_tile(aabb.max);

    for (i32 x = min_tile.x; x <= max_tile.x; x++)
    {
        for (i32 y = min_tile.y; y <= max_tile.y; y++)
        {
            PhysicsTileCoord tile_coord;
            tile_coord.x = x;
            tile_coord.y = y;
            (void)area.tiles_on.add(tile_coord);

            PhysicsTile& tile = _get_or_create_tile(tile_coord);
            bool can_insert = true;
            for (auto area_id : tile.areas)
            {
                if (area_id == area.self)
                {
                    can_insert = false;
                    break;
                }
            }

            if (can_insert)
                (void)tile.areas.add(area.self);
        }
    }
}

P2DDriver::PhysicsTile& P2DDriver::_get_or_create_tile(PhysicsTileCoord tile_coord)
{
    if (data.world_tiles.has(tile_coord))
    {
        return data.world_tiles.get(tile_coord);
    }
    
    PhysicsTile& new_tile = data.world_tiles.insert(tile_coord, PhysicsTile());
    new_tile.bodies = Array<Physics2D::BodyID>::with_size(get_allocator(), 16);
    new_tile.areas = Array<Physics2D::AreaID>::with_size(get_allocator(), 16);
    new_tile.coord = tile_coord;
    return new_tile;
}

