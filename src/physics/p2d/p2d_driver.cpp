#include "physics/p2d/p2d_driver.h"

#include "log/log.h"
#include "physics/physics_2d.h"
#include "physics/p2d/p2d_types.h"
#include "physics/p2d/p2d_collision.h"
#include "physics/p2d/p2d_body.h"
#include "physics/p2d/p2d_shape.h"



Physics2D::VTable P2DDriver::get_vtable()
{
    return Physics2D::VTable
    {
        .initialize = &P2DDriver::initialize,
        .shutdown = &P2DDriver::shutdown,

        .step = &P2DDriver::step,

        .body_create = &P2DDriver::body_create,
        .body_destroy = &P2DDriver::body_destroy,
        .area_create = &P2DDriver::area_create,
        .area_destroy = &P2DDriver::area_destroy,

        .body_set_shape = &P2DDriver::body_set_shape,
        .body_get_shape = &P2DDriver::body_get_shape,

        .body_set_user_data = &P2DDriver::body_set_user_data,
        .body_get_user_data = &P2DDriver::body_get_user_data,
        .body_set_transform = &P2DDriver::body_set_transform,
        .body_get_transform = &P2DDriver::body_get_transform,
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
        .body_set_restitution = &P2DDriver::body_set_restitution,
        .body_get_restitution = &P2DDriver::body_get_restitution,
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

        .area_set_user_data = &P2DDriver::area_set_user_data,
        .area_get_user_data = &P2DDriver::area_get_user_data,
        .area_set_transform = &P2DDriver::area_set_transform,
        .area_get_transform = &P2DDriver::area_get_transform,
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

    data.active_areas = Array<Physics2D::AreaID>::with_size(data.allocator, 4);
    data.active_bodies = Array<Physics2D::BodyID>::with_size(data.allocator, 4);

    data.current_bodies = FreeList<P2DBody, Physics2D::BodyID>::with_size(data.allocator, 4);
    data.current_areas = FreeList<P2DArea, Physics2D::AreaID>::with_size(data.allocator, 4);

    data.collision_callbacks_map = HashMap<CollisionID, CollisionCallback>::with_size(data.allocator, 4);
    data.resolved_pairs = HashMap<CollisionID, bool>::with_size(data.allocator, 16);

    data.tile_size = Physics2D::get_property("/tile_size").get<i32>();
	data.world_tiles = HashMap<PhysicsTileCoord, PhysicsTile>::with_size(data.allocator, InitialWorldTiles);

    data.fixed_step = Physics2D::get_property("/fixed_step").get<f32>();
    data.accumulator = 0.0f;
}

void P2DDriver::shutdown()
{
    data.active_areas.destroy();
    data.active_bodies.destroy();

    data.current_bodies.destroy();
    data.current_areas.destroy();
    data.collision_callbacks_map.destroy();
    data.resolved_pairs.destroy();

    for (auto& [cid, tile] : data.world_tiles.iter())
    {
        tile.bodies.destroy();
        tile.areas.destroy();
    }
    data.world_tiles.destroy();
}

void P2DDriver::step(f32 dt)
{
    data.accumulator += dt;
    while (data.accumulator >= data.fixed_step)
    {
        for (Physics2D::BodyID body_id : data.active_bodies.iter())
        {
            P2DBody& body = _get_body(body_id);
            body.is_on_floor = false;
            body.is_on_ceil = false;
        }

        const i32 sub_steps = 4;
        for(i32 i = 0; i < sub_steps; i++)
        {
            _step_fixed(data.fixed_step / sub_steps);
        }
        data.accumulator -= data.fixed_step;
    }

    for (Physics2D::BodyID body_id : data.active_bodies.iter())
    {
        P2DBody& body = _get_body(body_id);
        _handle_debug_draw_body(body);
    }

    for (Physics2D::AreaID area_id : data.active_areas.iter())
    {
        P2DArea& area = _get_area(area_id);
        _handle_debug_draw_area(area);
    }

    if (!data.debug_draw)
        return;

    //for (auto& [cid, tile] : data.world_tiles.iter())
    //{
    //    const PhysicsTileCoord& coord = cid;
    //    if(tile.bodies.count == 0 && tile.areas.count == 0)
    //        continue;

    //    const f32 ts = f32(_get_tile_size());
    //    Vector2 min = Vector2((coord.x) * ts, (coord.y) * ts);
    //    Vector2 max = min + Vector2(ts, ts);
    //    RenderManager::render_item_draw_line(data.grid_item, Vector2(min.x, min.y), Vector2(max.x, min.y), Color(128, 128, 128, 255));
    //    RenderManager::render_item_draw_line(data.grid_item, Vector2(max.x, min.y), Vector2(max.x, max.y), Color(128, 128, 128, 255));
    //    RenderManager::render_item_draw_line(data.grid_item, Vector2(max.x, max.y), Vector2(min.x, max.y), Color(128, 128, 128, 255));
    //    RenderManager::render_item_draw_line(data.grid_item, Vector2(min.x, max.y), Vector2(min.x, min.y), Color(128, 128, 128, 255));
    //}
}

Physics2D::BodyID P2DDriver::body_create(Opaque* user_data)
{
    Physics2D::BodyID id = data.current_bodies.add(P2DBody());
    P2DBody& new_body = data.current_bodies.get(id);
    (void)data.active_bodies.add(id);

    new_body.init(get_allocator(), id, user_data);

    return id;
}

void P2DDriver::body_destroy(Physics2D::BodyID body_id)
{
    P2DBody& body = _get_body(body_id);
    body_set_residence_mask(body_id, Physics2D::CollisionMask(0));
    body_set_collision_mask(body_id, Physics2D::CollisionMask(0));
    data.active_bodies.remove(body_id);

    // Removeing from a tile and areas
    for (PhysicsTileCoord tile_id : body.tiles_on.iter())
    {
        PhysicsTile& tile = _get_or_create_tile(tile_id);
        tile.bodies.remove(body_id);

        for(Physics2D::AreaID area_id : tile.areas.iter())
        {
            P2DArea& area = _get_area(area_id);
            if(!area.bodies_inside.has(body.self))
                continue;

            if(area.on_body_exit.has_func())
                area.on_body_exit.call(area.self, body.self);
            
            area.bodies_inside.remove(body.self);
        }
    }

    body.destroy();
    data.current_bodies.remove(body_id);
}

Physics2D::AreaID P2DDriver::area_create(Opaque* user_data)
{
    Physics2D::AreaID id = data.current_areas.add(P2DArea());
    P2DArea& new_area = data.current_areas.get(id);

    _active_area(id);

    new_area.init(get_allocator(), id, user_data);

    return id;
}

void P2DDriver::area_destroy(Physics2D::AreaID area_id)
{
    P2DArea& area = _get_area(area_id);
    _disable_area(area_id);

    // Removeing from a tile
    for (PhysicsTileCoord tile_id : area.tiles_on.iter())
    {
        PhysicsTile& tile = _get_or_create_tile(tile_id);
        tile.areas.remove(area.self);
    }

    // Exiting bodies
    for (auto& [body_id, value] : area.bodies_inside.iter())
    {
        P2DBody& body = _get_body(body_id);
        if(area.on_body_exit.has_func())
            area.on_body_exit.call(area.self, body.self);
    }

    area.destroy();

    data.current_areas.remove(area_id);
}

void P2DDriver::body_set_shape(Physics2D::BodyID body_id, const Shape2D& new_shape)
{
    P2DBody& body = _get_body(body_id);
    body.set_shape_from_2d(new_shape);
    _body_recompute_tiles(body);
}

Shape2D P2DDriver::body_get_shape(Physics2D::BodyID body_id)
{
    P2DBody& body = _get_body(body_id);
    return body.shape.to_shape_2d();
}

void P2DDriver::body_set_user_data(Physics2D::BodyID body_id, Opaque* user_data)
{
    P2DBody& body = _get_body(body_id);
    body.user_data = user_data;
}

Opaque* P2DDriver::body_get_user_data(Physics2D::BodyID body_id)
{
    P2DBody& body = _get_body(body_id);
    return body.user_data;
}

void P2DDriver::body_set_transform(Physics2D::BodyID body_id, const Transform2D& new_transform)
{
    P2DBody& body = _get_body(body_id);
    body.set_transform(new_transform);
}

Transform2D P2DDriver::body_get_transform(Physics2D::BodyID body_id)
{
    const P2DBody& body = _get_body(body_id);
    return body.get_transform();
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

void P2DDriver::body_set_restitution(Physics2D::BodyID body_id, f32 new_restitution)
{
    P2DBody& body = _get_body(body_id);
    body.set_restitution(new_restitution);
}

f32 P2DDriver::body_get_restitution(Physics2D::BodyID body_id)
{
    const P2DBody& body = _get_body(body_id);
    return body.get_restitution();
}

void P2DDriver::body_apply_force(Physics2D::BodyID body_id, const Vector2& force, const Vector2& point)
{
    P2DBody& body = _get_body(body_id);
    body.apply_force(force, point);
}

void P2DDriver::body_apply_impulse(Physics2D::BodyID body_id, const Vector2& impulse, const Vector2&)
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

void P2DDriver::body_set_on_collide(Physics2D::BodyID body_id, Physics2D::EventOnCollide on_collide)
{
    P2DBody& body = _get_body(body_id);
    body.on_collide = on_collide;
}

void P2DDriver::area_set_shape(Physics2D::AreaID area_id, const Shape2D& new_shape)
{
    P2DArea& area = _get_area(area_id);
    area.set_shape_from_2d(new_shape);

    // Getting tiles in area
    _area_recompute_tiles(area);
}

Shape2D P2DDriver::area_get_shape(Physics2D::AreaID area_id)
{
    P2DArea& area = _get_area(area_id);
    return area.shape.to_shape_2d();
}

void P2DDriver::area_set_user_data(Physics2D::AreaID area_id, Opaque* user_data)
{
    P2DArea& area = _get_area(area_id);
    area.user_data = user_data;
}

Opaque* P2DDriver::area_get_user_data(Physics2D::AreaID area_id)
{
    P2DArea& area = _get_area(area_id);
    return area.user_data;
}

void P2DDriver::area_set_transform(Physics2D::AreaID area_id, const Transform2D& new_transform)
{
    P2DArea& area = _get_area(area_id);
    area.set_transform(new_transform);
}

Transform2D P2DDriver::area_get_transform(Physics2D::AreaID area_id)
{
    P2DArea& area = _get_area(area_id);
    return area.get_transform();
}

void P2DDriver::area_set_residence_mask(Physics2D::AreaID area_id, Physics2D::CollisionMask mask)
{
    P2DArea& area = _get_area(area_id);
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
    P2DArea& area = _get_area(area_id);
    return area.residence_mask;
}

void P2DDriver::area_set_on_body_enter(Physics2D::AreaID area_id, Physics2D::EventOnBodyEnter on_body_enter)
{
    P2DArea& area = _get_area(area_id);
    area.on_body_enter = on_body_enter;
}

void P2DDriver::area_set_on_body_exit(Physics2D::AreaID area_id, Physics2D::EventOnBodyExit on_body_exit)
{
    P2DArea& area = _get_area(area_id);
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
    for (Physics2D::AreaID area_id : data.active_areas.iter())
    {
        P2DArea& area = _get_area(area_id);
        area.check_counter++;

        _area_recompute_tiles(area);
        _check_area_collision(area);
        _check_area_bodies_still_inside(area);
    }

    data.resolved_pairs.clear();

    for (Physics2D::BodyID body_id : data.active_bodies.iter())
    {
        P2DBody& body = _get_body(body_id);

        _move_body(body, dt);
        _body_recompute_tiles(body);
        _check_body_collision(body);
    }

    // Resolve pending static collisions
    for (Physics2D::BodyID body_id : data.active_bodies.iter())
    {
        P2DBody& body = _get_body(body_id);
        body.moved = false;
    }

    _resolve_collision_callbacks();
}

void P2DDriver::_handle_debug_draw_body(P2DBody& body)
{
    Unused(body);
    if (data.debug_draw == false)
        return;
}

void P2DDriver::_handle_debug_draw_area(P2DArea& area)
{
    Unused(area);
    if (data.debug_draw == false)
        return;
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
}

void P2DDriver::_check_body_collision(P2DBody& body)
{
    for (PhysicsTileCoord tile_coord : body.tiles_on.iter())
    {
        PhysicsTile& tile = _get_or_create_tile(tile_coord);
        _check_body_collisions_on_tile(body, tile);
    }
}

void P2DDriver::_check_body_collisions_on_tile(P2DBody& body, PhysicsTile& tile)
{
	const P2DShape& body_shape = body.get_shape_transformed();

    for (const Physics2D::BodyID other_body_id : tile.bodies.iter())
    {
        if (body.self == other_body_id)
            continue;

        P2DBody& other_body = _get_body(other_body_id);

        // Ignore collision between static objects.
        if(body.type == Physics2D::STATIC && other_body.type == Physics2D::STATIC)
            continue;

        if ((body.collision_mask & other_body.residence_mask) == 0)
            continue;

        const P2DShape& other_shape = other_body.get_shape_transformed();

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

        _body_solve_manifold(body, other_body, manifold);
    }
}

void P2DDriver::_body_solve_manifold(P2DBody& body, P2DBody& other_body, const CollisionManifold& manifold)
{
    if (!body.is_on_floor)
        body.is_on_floor = manifold.normal.y < 0;
    if (!body.is_on_ceil)
        body.is_on_ceil = manifold.normal.y > 0;

    if (!other_body.is_on_floor)
        other_body.is_on_floor = manifold.normal.y < 0;
    if (!other_body.is_on_ceil)
        other_body.is_on_ceil = manifold.normal.y > 0;

    CollisionID pair_id = CollisionID(body.self, other_body.self);
    if (!data.resolved_pairs.has(pair_id))
    {
        data.resolved_pairs.insert(pair_id, true);
        P2DCollision::positional_correction(manifold, body, other_body);
        P2DCollision::resolve_collision(manifold, body, other_body);
    }

    if (body.on_collide.has_func() || other_body.on_collide.has_func())
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

void P2DDriver::_check_area_collision(P2DArea& area)
{
    for (const PhysicsTileCoord tile_coord : area.tiles_on.iter())
    {
        PhysicsTile& tile = _get_or_create_tile(tile_coord);
        _check_area_collision_on_tile(area, tile);
    }
}

void P2DDriver::_check_area_collision_on_tile(P2DArea& area, PhysicsTile& tile)
{
    const P2DShape& area_shape = area.get_shape_transformed();

    for(const Physics2D::BodyID body_id : tile.bodies.iter())
    {
        P2DBody& body = _get_body(body_id);

        if ((area.residence_mask & body.residence_mask) == 0)
            continue;

        const P2DShape& other_shape = body.get_shape_transformed();

        bool collided = false;
        collided = area_shape.aabb.intersecs(other_shape.aabb);
        if(collided)
        {
            CollisionManifold manifold = P2DCollision::polygon_v_polygon(area_shape, other_shape);
            collided = manifold.valid;
        }

        _area_handle_collision(area, body, collided);
    }
}

void P2DDriver::_area_handle_collision(P2DArea& area, P2DBody& body, bool collided)
{
    // Is already inside
    bool has_body = area.bodies_inside.has(body.self);

    if(has_body && !collided)
    {
        area.bodies_inside.remove(body.self);
        if (area.on_body_exit.has_func())
            area.on_body_exit.call(area.self, body.self);
        return;
    }
    else if(has_body && collided)
    {
        P2DArea::BodyInArea& body_in_area = area.bodies_inside.get(body.self);
        body_in_area.check_counter = area.check_counter;
        return;
    }

    if(!collided)
        return;

    area.bodies_inside.insert(
        body.self,
        P2DArea::BodyInArea
        {
            .is_inside = true,
            .check_counter = area.check_counter,
        }
    );
    
    if (area.on_body_enter.has_func() == false)
        return;
    
    area.on_body_enter.call(area.self, body.self);
}

void P2DDriver::_check_area_bodies_still_inside(P2DArea& area)
{
    for(auto& [body_id, value] : area.bodies_inside.iter())
    {
        P2DBody& body = _get_body(body_id);
        if(value.check_counter != area.check_counter)
        {
            area.bodies_inside.remove(body_id);
            if (area.on_body_exit.has_func())
                area.on_body_exit.call(area.self, body.self);
        }
    }
}

void P2DDriver::_resolve_collision_callbacks()
{
    for (auto& [cid, value] : data.collision_callbacks_map.iter())
    {
        P2DBody& body = _get_body(value.body);
        P2DBody& collided = _get_body(value.collided);

        if (body.on_collide.has_func())
        {
            body.on_collide.call(body.self, collided.self);
        }
        if(collided.on_collide.has_func())
        {
            collided.on_collide.call(collided.self, body.self);
        }
    }

    data.collision_callbacks_map.clear();
}

void P2DDriver::_active_area(Physics2D::AreaID area_id)
{
    P2DArea& area = _get_area(area_id);
    
    if (area.is_active)
        return;

    area.is_active = true;
    (void)data.active_areas.add(area_id);
}

void P2DDriver::_disable_area(Physics2D::AreaID area_id)
{
    P2DArea& area = _get_area(area_id);
    if (!area.is_active)
        return;

    area.is_active = false;
    data.active_areas.remove(area_id);
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
    for (PhysicsTileCoord tile_id : body.tiles_on.iter())
    {
        PhysicsTile& tile = _get_or_create_tile(tile_id);
        tile.bodies.remove(body.self);
    }
    body.tiles_on.clear();

    // Getting tiles in shape
    const P2DShape& body_shape_transformed = body.get_shape_transformed();

    AABB aabb = body_shape_transformed.aabb;
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
            for (Physics2D::BodyID body_id : tile.bodies.iter())
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

    return;
}

void P2DDriver::_area_recompute_tiles(P2DArea& area)
{
    // Removing from old tiles
    for (PhysicsTileCoord tile_id : area.tiles_on.iter())
    {
        PhysicsTile& tile = _get_or_create_tile(tile_id);
        tile.areas.remove(area.self);
    }
    area.tiles_on.clear();

    // Getting tiles in shape
    const P2DShape& area_shape_transformed = area.get_shape_transformed();

    const AABB& aabb = area_shape_transformed.aabb;
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
            for (Physics2D::AreaID area_id : tile.areas.iter())
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

