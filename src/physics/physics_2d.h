#pragma once
#include "core/header.h"
#include "collections/event.h"
#include "collections/property.h"
#include "collections/string_map.h"
#include "mem/allocator.h"
#include "physics/shape_2d.h"


struct Object;
struct Object2D;


struct Physics2D
{
    enum DriverType
    {
        UNKNOWN_DRIVER = 0,
        
        P2D,

        DEFAULT_DRIVER = P2D,
    };

    using BodyID = ID<u32>;
    using AreaID = ID<u32>;

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
    using EventOnCollide = Event<void(*)(void*, Object2D*)>;
    using EventOnBodyEnter = Event<void(*)(void*, Object2D*)>;
    using EventOnBodyExit = Event<void(*)(void*, Object2D*)>;

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

        VTFunc(void, body_set_type, BodyID, BodyType);
        VTFunc(void, body_set_velocity, BodyID, const Vector2&);
        VTFunc(Vector2, body_get_velocity, BodyID);
        VTFunc(void, body_set_mass, BodyID, f32);
        VTFunc(f32, body_get_mass, BodyID);
        VTFunc(void, body_set_friction, BodyID, f32);
        VTFunc(f32, body_get_friction, BodyID);
        VTFunc(void, body_set_air_friction, BodyID, f32);
        VTFunc(f32, body_get_air_friction, BodyID);
        VTFunc(void, body_apply_force, BodyID, const Vector2&, const Vector2&);
        VTFunc(void, body_apply_impulse, BodyID, const Vector2&, const Vector2&);
        VTFunc(void, body_set_fixed_rotation, BodyID, bool);
        VTFunc(bool, body_is_on_floor, BodyID);
        VTFunc(bool, body_is_on_ceil, BodyID);
        VTFunc(void, body_set_residence_mask, BodyID, CollisionMask);
        VTFunc(CollisionMask, body_get_residence_mask, BodyID);
        VTFunc(void, body_set_collision_mask, BodyID, CollisionMask);
        VTFunc(CollisionMask, body_get_collision_mask, BodyID);
        VTFunc(void, body_set_on_collide, BodyID, void*, EventOnCollide);

        VTFunc(void, area_set_shape, AreaID, const Shape2D&);
        VTFunc(Shape2D, area_get_shape, AreaID);

        VTFunc(void, area_set_residence_mask, AreaID, CollisionMask);
        VTFunc(CollisionMask, area_get_residence_mask, AreaID);

        VTFunc(void, area_set_on_body_enter, AreaID, void*, EventOnBodyEnter);
        VTFunc(void, area_set_on_body_exit, AreaID, void*, EventOnBodyExit);

        // Internal
        VTFunc(void, property_change, StringView, PropertyValue);
    };

    struct InternalData
    {
        mem::Allocator allocator;

        StringMap<PropertyValue> properties;
    };

    static inline VTable vtable;
    static inline InternalData data{};

    static void initialize(const mem::Allocator& allocator, DriverType driver);
    static void shutdown();
    
    VTFuncDefArg1S(step, f32);
    
    VTFuncDefArg1RetS(BodyID, create_body, Object2D*);
    VTFuncDefArg1S(destroy_body, BodyID);
    VTFuncDefArg1RetS(AreaID, create_area, Object2D*);
    VTFuncDefArg1S(destroy_area, AreaID);

    VTFuncDefArg2S(body_set_shape, BodyID, const Shape2D&);
    VTFuncDefArg1RetS(Shape2D, body_get_shape, BodyID);

    VTFuncDefArg2S(body_set_type, BodyID, BodyType);
    VTFuncDefArg2S(body_set_velocity, BodyID, const Vector2&);
    VTFuncDefArg1RetS(Vector2, body_get_velocity, BodyID);
    VTFuncDefArg2S(body_set_mass, BodyID, f32);
    VTFuncDefArg1RetS(f32, body_get_mass, BodyID);
    VTFuncDefArg2S(body_set_friction, BodyID, f32);
    VTFuncDefArg1RetS(f32, body_get_friction, BodyID);
    VTFuncDefArg2S(body_set_air_friction, BodyID, f32);
    VTFuncDefArg1RetS(f32, body_get_air_friction, BodyID);
    VTFuncDefArg3S(body_apply_force, BodyID, const Vector2&, const Vector2&);
    VTFuncDefArg3S(body_apply_impulse, BodyID, const Vector2&, const Vector2&);
    VTFuncDefArg2S(body_set_fixed_rotation, BodyID, bool);
    VTFuncDefArg1RetS(bool, body_is_on_floor, BodyID);
    VTFuncDefArg1RetS(bool, body_is_on_ceil, BodyID);
    VTFuncDefArg2S(body_set_residence_mask, BodyID, CollisionMask);
    VTFuncDefArg1RetS(CollisionMask, body_get_residence_mask, BodyID);
    VTFuncDefArg2S(body_set_collision_mask, BodyID, CollisionMask);
    VTFuncDefArg1RetS(CollisionMask, body_get_collision_mask, BodyID);
    VTFuncDefArg3S(body_set_on_collide, BodyID, void*, EventOnCollide);

    VTFuncDefArg2S(area_set_shape, AreaID, const Shape2D&);
    VTFuncDefArg1RetS(Shape2D, area_get_shape, AreaID);

    VTFuncDefArg2S(area_set_residence_mask, AreaID, CollisionMask);
    VTFuncDefArg1RetS(CollisionMask, area_get_residence_mask, AreaID);
    VTFuncDefArg3S(area_set_on_body_enter, AreaID, void*, EventOnBodyEnter);
    VTFuncDefArg3S(area_set_on_body_exit, AreaID, void*, EventOnBodyExit);

    // Properties

    static void set_property(StringView property_name, PropertyValue new_value);
    static PropertyValue get_property(StringView property_name);
};
