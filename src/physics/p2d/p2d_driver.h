#pragma once
#include "collections/array.h"
#include "collections/hash_map.h"
#include "collections/free_list.h"
#include "physics/physics_2d_adapter.h"

#include "physics/p2d/p2d_area.h"
#include "physics/p2d/p2d_body.h"
#include "physics/p2d/p2d_types.h"



struct P2DDriver : InternalPhysics2D::Adapter
{
    static constexpr usize InitialWorldTiles = 64;

    struct CollisionCallback
    {
        Physics2D::BodyID body;
        Physics2D::BodyID collided;
    };

    struct PhysicsTile
    {
        PhysicsTileCoord coord;
        Array<Physics2D::BodyID> bodies;
        Array<Physics2D::AreaID> areas;
    };

    struct InternalData
    {
        Mem::Allocator* allocator;

        Vector2 gravity;

        Array<Physics2D::AreaID> active_areas;
        Array<Physics2D::BodyID> active_bodies;

        FreeList<P2DBody, Physics2D::BodyID> current_bodies;
        FreeList<P2DArea, Physics2D::AreaID> current_areas;

        HashMap<CollisionID, CollisionCallback> collision_callbacks_map;
        HashMap<CollisionID, bool> resolved_pairs;

        i32 tile_size;
        bool debug_draw;
        HashMap<PhysicsTileCoord, PhysicsTile> world_tiles;

        f32 fixed_step;
        f32 accumulator;
    };

    InternalData data;

    static InternalPhysics2D::Adapter get_vtable();

    [[nodiscard]] Mem::Allocator* get_allocator()
    {
        return data.allocator;
    }

    [[nodiscard]] P2DBody& _get_body(Physics2D::BodyID body_id)
    {
        return data.current_bodies.get(body_id);
    }

    [[nodiscard]] P2DArea& _get_area(Physics2D::AreaID area_id)
    {
        return data.current_areas.get(area_id);
    }

    [[nodiscard]] i32 _get_tile_size() { return data.tile_size; }

    void initialize(Mem::Allocator* allocator) override;
    void shutdown() override;

    void step(f32 dt) override;

    Physics2D::BodyID body_create(Opaque* user_data) override;
    void body_destroy(Physics2D::BodyID body_id) override;
    Physics2D::AreaID area_create(Opaque* user_data) override;
    void area_destroy(Physics2D::AreaID area_id) override;

    // Body
    void body_set_shape(Physics2D::BodyID body_id, const Shape2D& new_shape) override;
    Shape2D body_get_shape(Physics2D::BodyID body_id) override;

    void body_set_user_data(Physics2D::BodyID body_id, Opaque* user_data) override;
    Opaque* body_get_user_data(Physics2D::BodyID body_id) override;
    void body_set_transform(Physics2D::BodyID body_id, const Transform2D& new_transform) override;
    Transform2D body_get_transform(Physics2D::BodyID body_id) override;
    void body_set_type(Physics2D::BodyID body_id, Physics2D::BodyType new_type) override;
    void body_set_velocity(Physics2D::BodyID body_id, const Vector2& new_velocity) override;
    Vector2 body_get_velocity(Physics2D::BodyID body_id) override;
    void body_set_angular_velocity(Physics2D::BodyID body_id, f32 angular_velocity) override;
    f32 body_get_angular_velocity(Physics2D::BodyID body_id) override;
    void body_set_mass(Physics2D::BodyID body_id, f32 new_mass) override;
    f32 body_get_mass(Physics2D::BodyID body_id) override;
    void body_set_friction(Physics2D::BodyID body_id, f32 new_friction) override;
    f32 body_get_friction(Physics2D::BodyID body_id) override;
    void body_set_air_friction(Physics2D::BodyID body_id, f32 new_air_friction) override;
    f32 body_get_air_friction(Physics2D::BodyID body_id) override;
    void body_set_restitution(Physics2D::BodyID body_id, f32 new_restitution) override;
    f32 body_get_restitution(Physics2D::BodyID body_id) override;
    void body_apply_force(Physics2D::BodyID body_id, const Vector2& force, const Vector2& point) override;
    void body_apply_impulse(Physics2D::BodyID body_id, const Vector2& impulse, const Vector2& point) override;
    void body_set_fixed_rotation(Physics2D::BodyID body_id, bool enable) override;
    bool body_is_on_floor(Physics2D::BodyID body_id) override;
    bool body_is_on_ceil(Physics2D::BodyID body_id) override;

    void body_set_residence_mask(Physics2D::BodyID body_id, Physics2D::CollisionMask mask) override;
    Physics2D::CollisionMask body_get_residence_mask(Physics2D::BodyID body_id) override;
    void body_set_collision_mask(Physics2D::BodyID body_id, Physics2D::CollisionMask mask) override;
    Physics2D::CollisionMask body_get_collision_mask(Physics2D::BodyID body_id) override;
    void body_set_on_collide(Physics2D::BodyID body_id, Physics2D::EventOnCollide on_collide) override;

    // Area
    void area_set_shape(Physics2D::AreaID area_id, const Shape2D& new_shape) override;
    Shape2D area_get_shape(Physics2D::AreaID area_id) override;

    void area_set_user_data(Physics2D::AreaID area_id, Opaque* user_data) override;
    Opaque* area_get_user_data(Physics2D::AreaID area_id) override;
    void area_set_transform(Physics2D::AreaID area_id, const Transform2D& new_transform) override;
    Transform2D area_get_transform(Physics2D::AreaID area_id) override;
    void area_set_residence_mask(Physics2D::AreaID area_id, Physics2D::CollisionMask mask) override;
    Physics2D::CollisionMask area_get_residence_mask(Physics2D::AreaID area_id) override;

    void area_set_on_body_enter(Physics2D::AreaID area_id, Physics2D::EventOnBodyEnter on_body_enter) override;
    void area_set_on_body_exit(Physics2D::AreaID area_id, Physics2D::EventOnBodyExit on_body_exit) override;

    // Property
    void property_change(StringView property_name, PropertyValue new_value) override;

    // P2D Internal
    void _step_fixed(f32 dt);

    // Draw routines

    void _handle_debug_draw_body(P2DBody& body);
    void _handle_debug_draw_area(P2DArea& area);

    // Body routines

    void _move_body(P2DBody& body, f32 dt);
    void _check_body_collision(P2DBody& body);
    void _check_body_collisions_on_tile(P2DBody& body, PhysicsTile& tile);
    void _body_solve_manifold(P2DBody& body, P2DBody& other_body, const CollisionManifold& manifold);

    // Area routines
    void _check_area_collision(P2DArea& area);
    void _check_area_collision_on_tile(P2DArea& area, PhysicsTile& tile);
    void _area_handle_collision(P2DArea& area, P2DBody& body, bool collided);
    void _check_area_bodies_still_inside(P2DArea& area);

    // Collision callbacks
    void _resolve_collision_callbacks();

    // Utilities
    void _active_area(Physics2D::AreaID area_id);
    void _disable_area(Physics2D::AreaID area_id);

    [[nodiscard]] PhysicsTileCoord _convert_to_world_tile(const Vector2& point);

    void _body_recompute_tiles(P2DBody& body);
    void _area_recompute_tiles(P2DArea& area);
    PhysicsTile& _get_or_create_tile(PhysicsTileCoord tile_coord);
};
