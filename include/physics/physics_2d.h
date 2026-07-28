#pragma once
#include "collections/function.h"
#include "collections/property.h"
#include "collections/string_map.h"
#include "mem/allocator.h"
#include "math/transform_2d.h"
#include "physics/shape_2d.h"


struct Object;
struct Object2D;

namespace InternalPhysics2D
{
struct Adapter;
}

struct Physics2D
{
    enum class DriverType
    {
        Unknown = 0,
        
        P2D,

        Default = P2D,
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
    using EventOnCollide = Function<void(*)(BodyID, BodyID)>;
    using EventOnBodyEnter = Function<void(*)(AreaID, BodyID)>;
    using EventOnBodyExit = Function<void(*)(AreaID, BodyID)>;

    struct InternalData
    {
        Mem::Allocator* allocator;

        StringMap<PropertyValue> properties;
    };

    static inline InternalData data = {};

    static InternalPhysics2D::Adapter* get_adapter();
    
    static void initialize(Mem::Allocator* allocator, DriverType driver_type);
    static void initialize_from_adapter(InternalPhysics2D::Adapter* adapter);
    static void shutdown();

    static void step(f32 dt);

    static Physics2D::BodyID body_create(Opaque* user_data);
    static void body_destroy(Physics2D::BodyID body);

    // Body
    static void body_set_shape(Physics2D::BodyID body, const Shape2D& new_shape);
    static Shape2D body_get_shape(Physics2D::BodyID body);

    static void body_set_user_data(Physics2D::BodyID body, Opaque* new_user_data);
    static Opaque* body_get_user_data(Physics2D::BodyID body);
    static void body_set_transform(Physics2D::BodyID body, const Transform2D& new_transform);
    static Transform2D body_get_transform(Physics2D::BodyID body);
    static void body_set_type(Physics2D::BodyID body, Physics2D::BodyType new_type);
    static void body_set_velocity(Physics2D::BodyID body, const Vector2& new_velocity);
    static Vector2 body_get_velocity(Physics2D::BodyID body);
    static void body_set_angular_velocity(Physics2D::BodyID body, f32 angular_velocity);
    static f32 body_get_angular_velocity(Physics2D::BodyID body);
    static void body_set_mass(Physics2D::BodyID body, f32 new_mass);
    static f32 body_get_mass(Physics2D::BodyID body);
    static void body_set_friction(Physics2D::BodyID body, f32 new_friction);
    static f32 body_get_friction(Physics2D::BodyID body);
    static void body_set_air_friction(Physics2D::BodyID body, f32 new_air_friction);
    static f32 body_get_air_friction(Physics2D::BodyID body);
    static void body_set_restitution(Physics2D::BodyID body, f32 new_restitution);
    static f32 body_get_restitution(Physics2D::BodyID body);
    static void body_apply_force(Physics2D::BodyID body, const Vector2& force, const Vector2& point);
    static void body_apply_impulse(Physics2D::BodyID body, const Vector2& impulse, const Vector2& point);
    static void body_set_fixed_rotation(Physics2D::BodyID body, bool enable);
    static bool body_is_on_floor(Physics2D::BodyID body);
    static bool body_is_on_ceil(Physics2D::BodyID body);

    static void body_set_residence_mask(Physics2D::BodyID body_id, Physics2D::CollisionMask mask);
    static Physics2D::CollisionMask body_get_residence_mask(Physics2D::BodyID body_id);
    static void body_set_collision_mask(Physics2D::BodyID body_id, Physics2D::CollisionMask mask);
    static Physics2D::CollisionMask body_get_collision_mask(Physics2D::BodyID body_id);
    static void body_set_on_collide(Physics2D::BodyID body_id, Physics2D::EventOnCollide on_collide);

    // Area
    static Physics2D::AreaID area_create(Opaque* user_data);
    static void area_destroy(Physics2D::AreaID area);
    
    static void area_set_shape(Physics2D::AreaID area, const Shape2D& new_shape);
    static Shape2D area_get_shape(Physics2D::AreaID area);

    static void area_set_user_data(Physics2D::AreaID area, Opaque* new_user_data);
    static Opaque* area_get_user_data(Physics2D::AreaID area);
    static void area_set_transform(Physics2D::AreaID area, const Transform2D& new_transform);
    static Transform2D area_get_transform(Physics2D::AreaID area);
    static void area_set_residence_mask(Physics2D::AreaID area, Physics2D::CollisionMask mask);
    static Physics2D::CollisionMask area_get_residence_mask(Physics2D::AreaID area);

    static void area_set_on_body_enter(Physics2D::AreaID area, Physics2D::EventOnBodyEnter on_body_enter);
    static void area_set_on_body_exit(Physics2D::AreaID area, Physics2D::EventOnBodyExit on_body_exit);

    // Properties
    static void set_property(StringView property_name, PropertyValue new_value);
    static PropertyValue get_property(StringView property_name);
};
