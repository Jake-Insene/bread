#pragma once
#include "core/header.h"
#include "collections/event.h"
#include "collections/property.h"
#include "collections/string_map.h"
#include "mem/allocator.h"
#include "math/transform_2d.h"
#include "physics/shape_2d.h"


struct Object;
struct Object2D;
struct Transform2D;

namespace InternalPhysics2D
{
struct Adapter;
}

struct Physics2D
{
    enum DriverType
    {
        UNKNOWN_DRIVER = 0,
        
        P2D,

        DEFAULT_DRIVER = P2D,
    };

    using BodyID = ID<u32, struct __BodyTag>;
    using AreaID = ID<u32, struct __AreaTag>;

    enum BodyType
    {
        UNKNOWN = 0,

        STATIC,
        DYNAMIC,
        KINEMATIC,
    };

    enum
    {
        COLLISION_MASK_0 = Bit(0),
        COLLISION_MASK_1 = Bit(1),
        COLLISION_MASK_2 = Bit(2),
        COLLISION_MASK_3 = Bit(3),
        COLLISION_MASK_4 = Bit(4),
        COLLISION_MASK_5 = Bit(5),
        COLLISION_MASK_6 = Bit(6),
        COLLISION_MASK_7 = Bit(7),
        COLLISION_MASK_8 = Bit(8),
        COLLISION_MASK_9 = Bit(9),
        COLLISION_MASK_10 = Bit(10),
        COLLISION_MASK_11 = Bit(11),
        COLLISION_MASK_12 = Bit(12),
        COLLISION_MASK_13 = Bit(13),
        COLLISION_MASK_14 = Bit(14),
        COLLISION_MASK_15 = Bit(15),

        MAX_COLLISION_MASKS = 16,
        DEFAULT_COLLISION_MASK = COLLISION_MASK_0,
    };

    using CollisionMask = u32;
    using EventOnCollide = Event<void(*)(BodyID, BodyID)>;
    using EventOnBodyEnter = Event<void(*)(AreaID, BodyID)>;
    using EventOnBodyExit = Event<void(*)(AreaID, BodyID)>;

    struct InternalData
    {
        mem::Allocator* allocator;

        StringMap<PropertyValue> properties;
    };

    static inline InternalData data = {};

    static void initialize(mem::Allocator* allocator, DriverType driver);
    static void initialize_from_adapter(InternalPhysics2D::Adapter* adapter);
    static void shutdown();
    
    static InternalPhysics2D::Adapter* get_adapter();

    static void step(f32 dt);

    static BodyID body_create(Opaque* user_data);
    static void body_destroy(BodyID body_id);
    static AreaID area_create(Opaque* user_data);
    static void area_destroy(AreaID area_id);

    static void body_set_shape(BodyID body_id, const Shape2D& shape);
    static Shape2D body_get_shape(BodyID body_id);

    static void body_set_user_data(BodyID body_id, Opaque* user_data);
    static Opaque* body_get_user_data(BodyID body_id);
    static void body_set_transform(BodyID body_id, const Transform2D& transform);
    static Transform2D body_get_transform(BodyID body_id);
    static void body_set_type(BodyID body_id, BodyType type);
    static void body_set_velocity(BodyID body_id, const Vector2& velocity);
    static Vector2 body_get_velocity(BodyID body_id);
    static void body_set_angular_velocity(BodyID body_id, f32 angular_velocity);
    static f32 body_get_angular_velocity(BodyID body_id);
    static void body_set_mass(BodyID body_id, f32 mass);
    static f32 body_get_mass(BodyID body_id);
    static void body_set_friction(BodyID body_id, f32 friction);
    static f32 body_get_friction(BodyID body_id);
    static void body_set_air_friction(BodyID body_id, f32 air_friction);
    static f32 body_get_air_friction(BodyID body_id);
    static void body_set_restitution(BodyID body_id, f32 new_restitution);
    static f32 body_get_restitution(BodyID body_id);
    static void body_apply_force(BodyID body_id, const Vector2& force, const Vector2& point);
    static void body_apply_impulse(BodyID body_id, const Vector2& impulse, const Vector2& point);
    static void body_set_fixed_rotation(BodyID body_id, bool fixed_rotation);
    static bool body_is_on_floor(BodyID body_id);
    static bool body_is_on_ceil(BodyID body_id);
    static void body_set_residence_mask(BodyID body_id, CollisionMask mask);
    static CollisionMask body_get_residence_mask(BodyID body_id);
    static void body_set_collision_mask(BodyID body_id, CollisionMask mask);
    static CollisionMask body_get_collision_mask(BodyID body_id);
    static void body_set_on_collide(BodyID body_id, EventOnCollide event);

    static void area_set_shape(AreaID area_id, const Shape2D& shape);
    static Shape2D area_get_shape(AreaID area_id);

    static void area_set_user_data(AreaID area_id, Opaque* user_data);
    static Opaque* area_get_user_data(AreaID area_id);
    static void area_set_transform(AreaID area_id, const Transform2D& transform);
    static Transform2D area_get_transform(AreaID area_id);
    static void area_set_residence_mask(AreaID area_id, CollisionMask mask);
    static CollisionMask area_get_residence_mask(AreaID area_id);
    static void area_set_on_body_enter(AreaID area_id, EventOnBodyEnter event);
    static void area_set_on_body_exit(AreaID area_id, EventOnBodyExit event);

    // Properties

    static void set_property(StringView property_name, PropertyValue new_value);
    static PropertyValue get_property(StringView property_name);
};
