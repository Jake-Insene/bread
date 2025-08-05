#pragma once
#include "collections/array.h"
#include "collections/hash_map.h"
#include "collections/queue_array.h"
#include "physics/physics_2d.h"
#include "physics/shape_2d.h"


struct CollisionID
{
    Physics2D::BodyID id1;
    Physics2D::BodyID id2;
};

template<>
struct HashOfType<CollisionID>
{
    [[nodiscard]] static constexpr u64 hashfunc(const CollisionID& k)
    {
        return k.id1.id | (u64(k.id2.id) << 32);
    }

    [[nodiscard]] static constexpr bool compare(const CollisionID& k1, const CollisionID& k2)
    {
        return (k1.id1 == k2.id1 && k1.id2 == k2.id2) || (k1.id1 == k2.id2 && k1.id2 == k2.id1);
    }
};

struct P2DDriver
{
    struct CollisionCallback
    {
        bool two_ways;
        Physics2D::BodyID body;
        Object2D* collided;
    };

    struct [[nodiscard]] Body
    {
        Object2D* target;
        Physics2D::BodyID self;
        Physics2D::CollisionMask residence_mask;
        Physics2D::CollisionMask collision_mask;

        void* _this;
        Physics2D::EventOnCollide on_collide;

        Vector2 last_updated_pos;
        Array<Shape2D> shapes;
        Physics2D::BodyType type;
        Vector2 velocity;
        f32 mass;
        f32 friction;

        bool fixed_rotation;
        bool is_on_floor;
    };

    struct [[nodiscard]] Area
    {
        Object2D* target;
        Physics2D::AreaID self;
        Physics2D::CollisionMask residence_mask;

        void* _this;
        Physics2D::EventOnBodyEnter on_body_enter;
        Physics2D::EventOnBodyExit on_body_exit;

        Array<Shape2D> shapes;
    
        bool is_active;

        struct BodyInArea
        {
            bool is_inside;
        };

		HashMap<Physics2D::BodyID, BodyInArea> bodies_inside;
    };

    struct CollisionMaskGroup
    {
        bool active;
        Array<Physics2D::BodyID> bodies;
    };

    struct CollisionResult
    {
        Vector2 advance;
    };

    struct InternalData
    {
        mem::Allocator allocator;

        Vector2 gravity;

        CollisionMaskGroup mask_groups[Physics2D::MAX_COLLISION_MASKS];
        Array<Physics2D::AreaID> active_areas;

        QueueArray<Body, Physics2D::BodyID> current_bodies;
        QueueArray<Area, Physics2D::AreaID> current_areas;

        HashMap<CollisionID, CollisionCallback> collision_callbacks_map;
    };

    static inline InternalData data;

    static Physics2D::VTable get_vtable();

    [[nodiscard]] static mem::Allocator& get_allocator()
    {
        return data.allocator;
    }

    [[nodiscard]] static Body& _get_body(Physics2D::BodyID body_id)
    {
        return data.current_bodies.get(body_id);
    }

    [[nodiscard]] static Area& _get_area(Physics2D::BodyID area_id)
    {
        return data.current_areas.get(area_id);
    }

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

    static void step(f32 dt);

    static Physics2D::BodyID create_body(Object2D* object);
    static void destroy_body(Physics2D::BodyID body_id);
    static Physics2D::AreaID create_area(Object2D* object);
    static void destroy_area(Physics2D::AreaID area_id);

    // Body
    static void body_add_shape(Physics2D::BodyID body_id, const Shape2D& new_shape);
    static void body_remove_shape(Physics2D::BodyID body_id, usize index);
    static usize body_get_shape_count(Physics2D::BodyID body_id);
    static Shape2D body_get_shape(Physics2D::BodyID body_id, usize index);
    static void body_set_shape(Physics2D::BodyID body_id, usize index, const Shape2D& shape);

    static void body_set_type(Physics2D::BodyID body_id, Physics2D::BodyType new_type);
    static void body_set_velocity(Physics2D::BodyID body_id, const Vector2& new_velocity);
    static Vector2 body_get_velocity(Physics2D::BodyID body_id);
    static void body_set_mass(Physics2D::BodyID body_id, f32 new_mass);
    static f32 body_get_mass(Physics2D::BodyID body_id);
    static void body_set_friction(Physics2D::BodyID body_id, f32 new_friction);
    static f32 body_get_friction(Physics2D::BodyID body_id);
    static void body_apply_force(Physics2D::BodyID body_id, const Vector2& point, const Vector2& force);
    static void body_apply_impulse(Physics2D::BodyID body_id, const Vector2& point, const Vector2& force);
    static void body_set_fixed_rotation(Physics2D::BodyID body_id, bool enable);
    static bool body_is_on_floor(Physics2D::BodyID body_id);

    static void body_set_residence_mask(Physics2D::BodyID body_id, Physics2D::CollisionMask mask);
    static Physics2D::CollisionMask body_get_residence_mask(Physics2D::BodyID body_id);
    static void body_set_collision_mask(Physics2D::BodyID body_id, Physics2D::CollisionMask mask);
    static Physics2D::CollisionMask body_get_collision_mask(Physics2D::BodyID body_id);
    static void body_set_on_collide(Physics2D::BodyID body_id, void* _this, Physics2D::EventOnCollide on_collide);

    // Area
    static void area_add_shape(Physics2D::AreaID area_id, const Shape2D& new_shape);
    static void area_remove_shape(Physics2D::AreaID area_id, usize index);
    static usize area_get_shape_count(Physics2D::AreaID area_id);
    static Shape2D area_get_shape(Physics2D::AreaID area_id, usize index);
    static void area_set_shape(Physics2D::AreaID area_id, usize index, const Shape2D& shape);

    static void area_set_residence_mask(Physics2D::AreaID area_id, Physics2D::CollisionMask mask);
    static Physics2D::CollisionMask area_get_residence_mask(Physics2D::AreaID area_id);

    static void area_set_on_body_enter(Physics2D::AreaID area_id, void* _this, Physics2D::EventOnBodyEnter on_body_enter);
    static void area_set_on_body_exit(Physics2D::AreaID area_id, void* _this, Physics2D::EventOnBodyExit on_body_exit);

    // Internal

    // Body routines
    static void _handle_debug_draw_body(Body& body);
    static void _handle_debug_draw_area(Area& area);

    static void _step_body(Body& body, f32 dt);
    static void _check_collision_in_group(CollisionMaskGroup& group, Body& body,
        Vector2& velocity, CollisionResult& collision_result);
    static void _check_collision_on_body(Body& body, const Shape2D& body_shape, Body& other_body, Vector2& velocity,
        CollisionResult& collision_result);

    // Area routines
    static void _check_body_in_areas(Body& body);
    static void _check_body_in_area(const Vector2& area_position, Area& area, Body& body);
    static void _check_body_in_shape(const Shape2D& area_shape, Area& area, Body& body, const Vector2& body_position);

    // Collision callbacks
    static void _resolve_collision_callbacks();

    // Utilities
    static void _mask_group_add(Physics2D::BodyID body_id, usize group_index);
    static void _mask_group_remove(Physics2D::BodyID body_id, usize group_index);

    static void _active_area(Physics2D::AreaID area_id);
    static void _disable_area(Physics2D::AreaID area_id);
};
