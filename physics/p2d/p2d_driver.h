#pragma once
#include "physics/physics_2d.h"
#include "physics/p2d/shape_2d.h"

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
        return k.id1.id | k.id2.id;
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
        Body2D* b1;
        Body2D* b2;
    };

    struct [[nodiscard]] Body
    {
        Body2D* target;
        Physics2D::BodyID self;
        Physics2D::CollisionMask residence_mask;
        Physics2D::CollisionMask collision_mask;
        Shape2D shape;

        Vector2 last_updated_pos;

        Physics2D::BodyType type;
        Vector2 velocity;
        f32 mass;
        f32 friction;

        u32 fixed_rotation : 1;
        u32 is_on_floor : 1;
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

        QueueArray<Body, Physics2D::BodyID> current_bodies;

        HashMap<CollisionID, CollisionCallback> collision_callbacks_map;
    };

    static inline InternalData data;

    static Physics2D::VTable get_vtable();

    [[nodiscard]] static mem::Allocator& get_allocator()
    {
        return data.allocator;
    }
    [[nodiscard]] static Body& _get_body(Physics2D::BodyID bodyid)
    {
        return data.current_bodies.get(bodyid);
    }

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

    static void step(f32 dt);

    static Physics2D::BodyID create_body(Body2D* object);
    static void destroy_body(Physics2D::BodyID body_id);

    static void body_as_box(Physics2D::BodyID body_id, const Vector2& new_size);
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

    static void _step_body(Body& body, f32 dt);
    static void _check_collision_in_group(CollisionMaskGroup& group, Body& body,
        Vector2& velocity, CollisionResult& collision_result);
    static void _resolve_collision_callbacks();

    static void _mask_group_add(Physics2D::BodyID body_id, usize group_index);
    static void _mask_group_remove(Physics2D::BodyID body_id, usize group_index);
};
