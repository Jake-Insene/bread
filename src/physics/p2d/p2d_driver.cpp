#include "physics/p2d/p2d_driver.h"

#include "graphics/viewport.h"
#include "physics/area_2d.h"
#include "physics/body_2d.h"
#include "2d/tile_map.h"



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
        .body_set_mass = &P2DDriver::body_set_mass,
        .body_get_mass = &P2DDriver::body_get_mass,
        .body_set_friction = &P2DDriver::body_set_friction,
        .body_get_friction = &P2DDriver::body_get_friction,
        .body_set_air_friction = &P2DDriver::body_set_air_friction,
        .body_get_air_friction = &P2DDriver::body_get_air_friction,
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

    data.current_bodies = FreeList<Body, Physics2D::BodyID>::with_size(data.allocator, 4);
    data.current_areas = FreeList<Area, Physics2D::AreaID>::with_size(data.allocator, 4);

    data.collision_callbacks_map = HashMap<CollisionID, CollisionCallback>::with_size(data.allocator, 4);

    data.tile_size = Physics2D::get_property("/tile_size").get<i32>();
	data.world_tiles = HashMap<PhysicsTileCoord, PhysicsTile>::with_size(data.allocator, InitialWorldTiles);
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

    for (auto& entry : data.world_tiles.entries)
    {
        if(entry)
        {
            entry->kv.second.bodies.destroy();
        }
    }
    data.world_tiles.destroy();
}

void P2DDriver::step(f32 dt)
{
    for (auto area_id : data.active_areas)
    {
        Area& area = _get_area(area_id);
        _area_recompute_tiles(area);
        _check_area_collision(area);
        _handle_debug_draw_area(area);
    }

    for(auto body_id : data.active_bodies)
    {
        Body& body = _get_body(body_id);
        _move_body(body, dt);
        _body_recompute_tiles(body);
        _check_body_collision(body);
        _handle_debug_draw_body(_get_body(body_id));
    }

    _resolve_collision_callbacks();

    for (auto& body_id : data.active_bodies)
    {
        Body& body = _get_body(body_id);
        body.moved = false;
    }
}

Physics2D::BodyID P2DDriver::create_body(Object2D* object)
{
    Physics2D::BodyID id = data.current_bodies.add(Body());
    Body& new_body = data.current_bodies.get(id);

    new_body.target = object;
    new_body.self = id;
    new_body.residence_mask = Physics2D::CollisionMask(Physics2D::DEFAULT_COLLISION_MASK);
    _mask_group_add(id, 0);
    (void)data.active_bodies.add(id);

    new_body.collision_mask = Physics2D::CollisionMask(Physics2D::DEFAULT_COLLISION_MASK);

    new_body.shape = P2DShape();
    new_body.tiles_on = Array<PhysicsTileCoord>::with_size(get_allocator(), 4);

    return id;
}

void P2DDriver::destroy_body(Physics2D::BodyID body_id)
{
    Body& body = _get_body(body_id);
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
    Body& body = _get_body(body_id);
    body.shape = P2DShape::from_shape_2d(shape);
    _body_recompute_tiles(body);
}

Shape2D P2DDriver::body_get_shape(Physics2D::BodyID body_id)
{
    Body& body = _get_body(body_id);
    return body.shape.to_shape_2d();
}

void P2DDriver::body_set_type(Physics2D::BodyID body_id, Physics2D::BodyType new_type)
{
    Body& body = _get_body(body_id);
    body.type = new_type;
    switch (new_type)
    {
    case Physics2D::STATIC:
        body.mass = 0;
        body.velocity_input = Vector2();
        break;
    case Physics2D::KINEMATIC:
        body.mass = 0;
        break;
    case Physics2D::DYNAMIC:
        body.mass = body.mass > 0 ? body.mass : 1;
        break;
    default:
        break;
    }
}

void P2DDriver::body_set_velocity(Physics2D::BodyID body_id, const Vector2& new_velocity)
{
    Body& body = _get_body(body_id);
    body.velocity_input = new_velocity;
}

Vector2 P2DDriver::body_get_velocity(Physics2D::BodyID body_id)
{
    Body& body = _get_body(body_id);
    return body.velocity_input;
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

void P2DDriver::body_set_air_friction(Physics2D::BodyID body_id, f32 new_air_friction)
{
    Body& body = _get_body(body_id);
    body.air_friction = new_air_friction;
}

f32 P2DDriver::body_get_air_friction(Physics2D::BodyID body_id)
{
    const Body& body = _get_body(body_id);
    return body.air_friction;
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

bool P2DDriver::body_is_on_ceil(Physics2D::BodyID body_id)
{
    const Body& body = _get_body(body_id);
    return body.is_on_ceil;
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
}

void P2DDriver::_handle_debug_draw_body(Body& body)
{
#if defined(ENABLE_DEBUG_OPTIONS)
    if (Physics2D::get_property("/debug_draw").get<bool>() == false)
        return;

    P2DShape shape = body.shape;
    Transform2D transform = body.target->get_global_transform();
    shape.apply_transform(transform);

    for(usize i = 0; i < 4; i++)
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
        transform * shape.get_centroid(), 1.f, Color(255, 0, 0, 255)
    );

#endif
}

void P2DDriver::_handle_debug_draw_area(Area& area)
{
#if defined(ENABLE_DEBUG_OPTIONS)
    if (Physics2D::get_property("/debug_draw").get<bool>() == false)
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
        transform * shape.get_centroid(), 1.f, Color(255, 0, 0, 255)
    );
#endif
}

void P2DDriver::_move_body(Body& body, f32 dt)
{
    if (body.type == Physics2D::STATIC)
        return;

    if (body.moved)
        return;

    body.moved = true;

    switch (body.type)
    {
    case Physics2D::STATIC:
        return;
    case Physics2D::KINEMATIC:
    {
        // TODO: This is only for ForYourGrace, remove when the physics engine can handle top down.
        body.velocity = body.velocity_input;
        body.force = Vector2();
    }
        break;
    case Physics2D::DYNAMIC:
    {
        if (!body.is_on_floor)
            body.force += data.gravity * body.mass;
        body.force += body.velocity_input;

        const Vector2 acceleration = body.force / body.mass;
        body.velocity += acceleration * dt;

        if (body.is_on_floor)
        {
            // For now cancel the input velocity on x
            if (body.velocity_input.x == 0.f)
            {
                body.velocity.x = 0.f;
            }
            else
            {
                body.velocity.x = math::move_to(body.velocity.x, 0.f, body.friction * dt);
            }

            if (body.velocity.y > 0.f)
            {
                // stop bouncing up
                body.velocity.y = 0.f;
            }
            else if (body.velocity.y < 0.f)
            {
                body.velocity.y = data.gravity.y * dt;
            }
        }
        else
        {
            body.velocity.x = math::move_to(body.velocity.x, 0.0f, body.air_friction * dt);
            body.velocity.y = math::move_to(body.velocity.y, 0.0f, body.air_friction * dt);
        }

        if ((body.velocity_input.x > 0.f && body.velocity.x > body.velocity_input.x)
            || (body.velocity_input.x < 0.f && body.velocity.x < body.velocity_input.x))
        {
            body.velocity.x = body.velocity_input.x;
        }

        if (math::abs(body.velocity.x) < 0.01f)
            body.velocity.x = 0.0f;
        if (math::abs(body.velocity.y) < 0.01f)
            body.velocity.y = 0.0f;

        body.force = Vector2();
    }
        break;
    default:
        return;
    }
    
    body.is_on_floor = false;
    body.is_on_ceil = false;

    body.target->translate(body.velocity * dt);
}

void P2DDriver::_check_body_collision(Body& body)
{
    for (auto tile_coord : body.tiles_on)
    {
        PhysicsTile& tile = _get_or_create_tile(tile_coord);
        _check_body_collisions_on_tile(body, tile);
    }
}

void P2DDriver::_check_body_collisions_on_tile(Body& body, PhysicsTile& tile)
{
    P2DShape body_shape = body.shape;
    body_shape.apply_transform(body.target->get_global_transform());

    for (auto other_body_id : tile.bodies)
    {
        if (body.self == other_body_id)
            continue;

        Body& other_body = _get_body(other_body_id);
        if ((body.collision_mask & other_body.residence_mask) == 0)
            continue;

        P2DShape other_shape = other_body.shape;
        other_shape.apply_transform(other_body.target->get_global_transform());

        P2DCollision::CollisionManifold manifold = P2DCollision::polygon_v_polygon(body_shape, other_shape);
        if (!manifold.valid)
            continue;

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
        Body& body = _get_body(body_id);
        if ((area.residence_mask & body.residence_mask) == 0)
            continue;

        P2DShape other_shape = body.shape;
        other_shape.apply_transform(body.target->get_global_transform());

        P2DCollision::CollisionManifold manifold = P2DCollision::polygon_v_polygon(area_shape, other_shape);
        if (manifold.valid)
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
        Body& body = _get_body(value.body);
        Body& collided = _get_body(value.collided);
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

static constexpr PhysicsTileCoord get_tile_coord(Vector2 point, Vector2 tile_size)
{
    const Vector2 normalized_point = point / tile_size;
    PhysicsTileCoord tile = {};

    tile.x = math::floor(normalized_point.x);
    if (tile.x < 0)
        tile.x += 1;

    tile.y = math::floor(normalized_point.y);
    if (tile.y < 0)
        tile.y += 1;

    return tile;
}

static constexpr auto tile00 = get_tile_coord(Vector2(), Vector2(64, 64));
static constexpr auto tile0_m1 = get_tile_coord(Vector2(0, -200), Vector2(64, 64));

PhysicsTileCoord P2DDriver::_convert_to_world_tile(const Vector2& point)
{
    const Vector2 normalized_point = point / f32(_get_tile_size());
    PhysicsTileCoord tile = {};

    tile.x = math::floor(normalized_point.x);
    if (tile.x < 0)
        tile.x += 1;

    tile.y = math::floor(normalized_point.y);
    if (tile.y < 0)
        tile.y += 1;

    return tile;
}


void P2DDriver::_body_recompute_tiles(Body& body)
{
    // Removing from a tiles
    for (auto& tile_id : body.tiles_on)
    {
        PhysicsTile& tile = _get_or_create_tile(tile_id);
        tile.bodies.remove_equal(body.self);
    }
    body.tiles_on.clear();

    // Getting tiles in shape
    P2DShape body_shape = body.shape;
    body_shape.translate(body.target->get_global_transform().get_position());

    for (auto vertice : body_shape.vertices)
    {
        PhysicsTileCoord tile_coord = _convert_to_world_tile(vertice);
        bool can_insert_tile_coord = true;
        for (auto tile_on : body.tiles_on)
        {
            if (tile_on == tile_coord)
            {
                can_insert_tile_coord = false;
            }
        }

        if(can_insert_tile_coord)
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

        if(can_insert)
            (void)tile.bodies.add(body.self);
    }
}

void P2DDriver::_area_recompute_tiles(Area& area)
{
    area.tiles_on.clear();

    // Getting tiles in shape
    P2DShape body_shape = area.shape;
    body_shape.apply_transform(area.target->get_global_transform());

    for (auto vertice : body_shape.vertices)
    {
        PhysicsTileCoord tile_coord = _convert_to_world_tile(vertice);
        bool can_insert_tile_coord = true;
        for (auto tile_on : area.tiles_on)
        {
            if (tile_on == tile_coord)
            {
                can_insert_tile_coord = false;
            }
        }

        if (can_insert_tile_coord)
            (void)area.tiles_on.add(tile_coord);

        PhysicsTile& tile = _get_or_create_tile(tile_coord);
        bool can_insert = true;
        for (auto body_id : tile.bodies)
        {
            if (body_id == area.self)
            {
                can_insert = false;
                break;
            }
        }

        if (can_insert)
            (void)tile.bodies.add(area.self);
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
    new_tile.coord = tile_coord;
    return new_tile;
}

