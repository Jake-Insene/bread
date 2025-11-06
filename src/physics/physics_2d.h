#pragma once
#include "core/header.h"
#include "collections/event.h"
#include "collections/property.h"
#include "collections/string_map.h"
#include "mem/allocator.h"
#include "physics/shape_2d.h"


struct Object;
struct Object2D;
struct Transform2D;


struct Physics2D
{
    struct __BodyTag {};
    struct __AreaTag {};

    enum DriverType
    {
        UNKNOWN_DRIVER = 0,
        
        P2D,

        DEFAULT_DRIVER = P2D,
    };

    using BodyID = ID<u32, __BodyTag>;
    using AreaID = ID<u32, __AreaTag>;

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
    using EventOnCollide = Event<void(*)(Opaque*, Object2D*)>;
    using EventOnBodyEnter = Event<void(*)(Opaque*, Object2D*)>;
    using EventOnBodyExit = Event<void(*)(Opaque*, Object2D*)>;

    struct VTable
    {
        VTFunc(void, initialize, const mem::Allocator&);
        VTFunc(void, shutdown);

        VTFunc(void, step, f32);

        VTFunc(BodyID, create_body, Object2D*);
        VTFunc(void, destroy_body, BodyID);
        VTFunc(AreaID, create_area, Object2D*);
        VTFunc(void, destroy_area, AreaID);

        VTFunc(void, body_set_shape, BodyID, const Shape2D&);
        VTFunc(Shape2D, body_get_shape, BodyID);

        VTFunc(void, body_set_transform, BodyID, const Transform2D&);
        VTFunc(void, body_set_type, BodyID, BodyType);
        VTFunc(void, body_set_velocity, BodyID, const Vector2&);
        VTFunc(Vector2, body_get_velocity, BodyID);
        VTFunc(void, body_set_angular_velocity, BodyID, f32);
        VTFunc(f32, body_get_angular_velocity, BodyID);
        VTFunc(void, body_set_mass, BodyID, f32);
        VTFunc(f32, body_get_mass, BodyID);
        VTFunc(void, body_set_friction, BodyID, f32);
        VTFunc(f32, body_get_friction, BodyID);
        VTFunc(void, body_set_air_friction, BodyID, f32);
        VTFunc(f32, body_get_air_friction, BodyID);
        VTFunc(void, body_set_bounce, BodyID, f32);
        VTFunc(f32, body_get_bounce, BodyID);
        VTFunc(void, body_apply_force, BodyID, const Vector2&, const Vector2&);
        VTFunc(void, body_apply_impulse, BodyID, const Vector2&, const Vector2&);
        VTFunc(void, body_set_fixed_rotation, BodyID, bool);
        VTFunc(bool, body_is_on_floor, BodyID);
        VTFunc(bool, body_is_on_ceil, BodyID);
        VTFunc(void, body_set_residence_mask, BodyID, CollisionMask);
        VTFunc(CollisionMask, body_get_residence_mask, BodyID);
        VTFunc(void, body_set_collision_mask, BodyID, CollisionMask);
        VTFunc(CollisionMask, body_get_collision_mask, BodyID);
        VTFunc(void, body_set_on_collide, BodyID, Opaque*, EventOnCollide);

        VTFunc(void, area_set_shape, AreaID, const Shape2D&);
        VTFunc(Shape2D, area_get_shape, AreaID);

        VTFunc(void, area_set_transform, AreaID, const Transform2D&);
        VTFunc(void, area_set_residence_mask, AreaID, CollisionMask);
        VTFunc(CollisionMask, area_get_residence_mask, AreaID);

        VTFunc(void, area_set_on_body_enter, AreaID, Opaque*, EventOnBodyEnter);
        VTFunc(void, area_set_on_body_exit, AreaID, Opaque*, EventOnBodyExit);

        // Internal
        VTFunc(void, property_change, StringView, PropertyValue);
    };

    struct InternalData
    {
        mem::Allocator allocator;

        StringMap<PropertyValue> properties;
    };

    static inline InternalData data{};

    static void initialize(const mem::Allocator& allocator, DriverType driver);
    static void shutdown();

    static void step(f32 dt);

    static BodyID create_body(Object2D* object);
    static void destroy_body(BodyID body_id);
    static AreaID create_area(Object2D* object);
    static void destroy_area(AreaID area_id);

    static void body_set_shape(BodyID body_id, const Shape2D& shape);
    static Shape2D body_get_shape(BodyID body_id);

    static void body_set_transform(BodyID body_id, const Transform2D& transform);
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
    static void body_set_bounce(BodyID body_id, f32 bounce);
    static f32 body_get_bounce(BodyID body_id);
    static void body_apply_force(BodyID body_id, const Vector2& force, const Vector2& position);
    static void body_apply_impulse(BodyID body_id, const Vector2& impulse, const Vector2& position);
    static void body_set_fixed_rotation(BodyID body_id, bool fixed_rotation);
    static bool body_is_on_floor(BodyID body_id);
    static bool body_is_on_ceil(BodyID body_id);
    static void body_set_residence_mask(BodyID body_id, CollisionMask mask);
    static CollisionMask body_get_residence_mask(BodyID body_id);
    static void body_set_collision_mask(BodyID body_id, CollisionMask mask);
    static CollisionMask body_get_collision_mask(BodyID body_id);
    static void body_set_on_collide(BodyID body_id, Opaque* user_data, EventOnCollide event);

    static void area_set_shape(AreaID area_id, const Shape2D& shape);
    static Shape2D area_get_shape(AreaID area_id);

    static void area_set_transform(AreaID area_id, const Transform2D& transform);
    static void area_set_residence_mask(AreaID area_id, CollisionMask mask);
    static CollisionMask area_get_residence_mask(AreaID area_id);
    static void area_set_on_body_enter(AreaID area_id, Opaque* user_data, EventOnBodyEnter event);
    static void area_set_on_body_exit(AreaID area_id, Opaque* user_data, EventOnBodyExit event);

    // Properties

    static void set_property(StringView property_name, PropertyValue new_value);
    static PropertyValue get_property(StringView property_name);
};
