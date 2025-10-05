#pragma once
#include "collections/array.h"
#include "collections/hash_map.h"
#include "collections/free_list.h"
#include "physics/physics_2d.h"

#include "physics/p2d/p2d_body.h"
#include "physics/p2d/p2d_collision.h"
#include "physics/p2d/p2d_shape.h"
#include "physics/p2d/p2d_types.h"



struct P2DDriver
{
    static constexpr usize InitialWorldTiles = 64;

    struct CollisionCallback
    {
        Physics2D::BodyID body;
        Physics2D::BodyID collided;
    };

    struct [[nodiscard]] Area
    {
        Object2D* target;
        Physics2D::AreaID self;
        Physics2D::CollisionMask residence_mask;

        void* _this;
        Physics2D::EventOnBodyEnter on_body_enter;
        Physics2D::EventOnBodyExit on_body_exit;

        P2DShape shape;

        bool is_active;

        struct BodyInArea
        {
            bool is_inside;
        };

		HashMap<Physics2D::BodyID, BodyInArea> bodies_inside;
        Array<PhysicsTileCoord> tiles_on;
    };

    struct CollisionMaskGroup
    {
        bool active;
        Array<Physics2D::BodyID> bodies;
    };

    struct PhysicsTile
    {
        PhysicsTileCoord coord;
        Array<Physics2D::BodyID> bodies;
    };

    struct InternalData
    {
        mem::Allocator allocator;

        Vector2 gravity;

        CollisionMaskGroup mask_groups[Physics2D::MAX_COLLISION_MASKS];
        Array<Physics2D::AreaID> active_areas;
        Array<Physics2D::BodyID> active_bodies;

        FreeList<P2DBody, Physics2D::BodyID> current_bodies;
        FreeList<Area, Physics2D::AreaID> current_areas;

        HashMap<CollisionID, CollisionCallback> collision_callbacks_map;

        i32 tile_size;
        bool debug_draw;
        HashMap<PhysicsTileCoord, PhysicsTile> world_tiles;
    };

    static inline InternalData data;

    static Physics2D::VTable get_vtable();

    [[nodiscard]] static mem::Allocator& get_allocator()
    {
        return data.allocator;
    }

    [[nodiscard]] static P2DBody& _get_body(Physics2D::BodyID body_id)
    {
        return data.current_bodies.get(body_id);
    }

    [[nodiscard]] static Area& _get_area(Physics2D::BodyID area_id)
    {
        return data.current_areas.get(area_id);
    }

    [[nodiscard]] static i32 _get_tile_size() { return data.tile_size; }

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

    static void step(f32 dt);

    static Physics2D::BodyID create_body(Object2D* object);
    static void destroy_body(Physics2D::BodyID body_id);
    static Physics2D::AreaID create_area(Object2D* object);
    static void destroy_area(Physics2D::AreaID area_id);

    // Body
    static void body_set_shape(Physics2D::BodyID body_id, const Shape2D& shape);
    static Shape2D body_get_shape(Physics2D::BodyID body_id);

    static void body_set_type(Physics2D::BodyID body_id, Physics2D::BodyType new_type);
    static void body_set_velocity(Physics2D::BodyID body_id, const Vector2& new_velocity);
    static Vector2 body_get_velocity(Physics2D::BodyID body_id);
    static void body_set_angular_velocity(Physics2D::BodyID body_id, f32 angular_velocity);
    static f32 body_get_angular_velocity(Physics2D::BodyID body_id);
    static void body_set_mass(Physics2D::BodyID body_id, f32 new_mass);
    static f32 body_get_mass(Physics2D::BodyID body_id);
    static void body_set_friction(Physics2D::BodyID body_id, f32 new_friction);
    static f32 body_get_friction(Physics2D::BodyID body_id);
    static void body_set_air_friction(Physics2D::BodyID body_id, f32 new_air_friction);
    static f32 body_get_air_friction(Physics2D::BodyID body_id);
    static void body_set_bounce(Physics2D::BodyID body_id, f32 new_bounce);
    static f32 body_get_bounce(Physics2D::BodyID body_id);
    static void body_apply_force(Physics2D::BodyID body_id, const Vector2& point, const Vector2& force);
    static void body_apply_impulse(Physics2D::BodyID body_id, const Vector2& point, const Vector2& force);
    static void body_set_fixed_rotation(Physics2D::BodyID body_id, bool enable);
    static bool body_is_on_floor(Physics2D::BodyID body_id);
    static bool body_is_on_ceil(Physics2D::BodyID body_id);

    static void body_set_residence_mask(Physics2D::BodyID body_id, Physics2D::CollisionMask mask);
    static Physics2D::CollisionMask body_get_residence_mask(Physics2D::BodyID body_id);
    static void body_set_collision_mask(Physics2D::BodyID body_id, Physics2D::CollisionMask mask);
    static Physics2D::CollisionMask body_get_collision_mask(Physics2D::BodyID body_id);
    static void body_set_on_collide(Physics2D::BodyID body_id, void* _this, Physics2D::EventOnCollide on_collide);

    // Area
    static void area_set_shape(Physics2D::AreaID area_id, const Shape2D& shape);
    static Shape2D area_get_shape(Physics2D::AreaID area_id);

    static void area_set_residence_mask(Physics2D::AreaID area_id, Physics2D::CollisionMask mask);
    static Physics2D::CollisionMask area_get_residence_mask(Physics2D::AreaID area_id);

    static void area_set_on_body_enter(Physics2D::AreaID area_id, void* _this, Physics2D::EventOnBodyEnter on_body_enter);
    static void area_set_on_body_exit(Physics2D::AreaID area_id, void* _this, Physics2D::EventOnBodyExit on_body_exit);

    // Property
    static void property_change(StringView property_name, PropertyValue new_value);

    // P2D Internal

    // Draw routines

    static void _handle_debug_draw_body(P2DBody& body);
    static void _handle_debug_draw_area(Area& area);

    // Body routines

    static void _move_body(P2DBody& body, f32 dt);
    static void _check_body_collision(P2DBody& body);
    static void _check_body_collisions_on_tile(P2DBody& body, PhysicsTile& tile);

    // Area routines
    static void _check_area_collision(Area& area);
    static void _check_area_collision_on_tile(Area& area, PhysicsTile& tile);
    
    // Collision callbacks
    static void _resolve_collision_callbacks();

    // Utilities
    static void _mask_group_add(Physics2D::BodyID body_id, usize group_index);
    static void _mask_group_remove(Physics2D::BodyID body_id, usize group_index);

    static void _active_area(Physics2D::AreaID area_id);
    static void _disable_area(Physics2D::AreaID area_id);

    [[nodiscard]] static PhysicsTileCoord _convert_to_world_tile(const Vector2& point);

    static void _body_recompute_tiles(P2DBody& body);
    static void _area_recompute_tiles(Area& area);
    static PhysicsTile& _get_or_create_tile(PhysicsTileCoord tile_coord);
};
