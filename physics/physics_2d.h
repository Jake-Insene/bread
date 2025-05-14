#pragma once
#include "core/header.h"
#include "mem/allocator.h"
#include "math/vec2.h"


struct Body2D;

struct Physics2D
{
    enum DriverType
    {
        UNKNOWN_DRIVER = 0,
        
        P2D,

        DEFAULT_DRIVER = P2D,
    };

    using BodyID = ID<u32>;

    enum BodyType
    {
        UNKNOWN = 0,

        STATIC,
        DYNAMIC,
        KINEMATIC,
    };

    enum CollisionMask
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
    };

    enum Physics2DSetting
    {
        NONE = 0,
        DEBUG_DRAW,
    };

    struct VTable
    {
        VTFunc(void, initialize, const mem::Allocator&);
        VTFunc(void, shutdown);

        VTFunc(void, step, f32);

        VTFunc(BodyID, create_body, Body2D*);
        VTFunc(void, destroy_body, BodyID);

        VTFunc(void, body_as_box, BodyID, const Vector2&);
        VTFunc(void, body_set_type, BodyID, BodyType);
        VTFunc(void, body_set_velocity, BodyID, const Vector2&);
        VTFunc(Vector2, body_get_velocity, BodyID);
        VTFunc(void, body_set_mass, BodyID, f32);
        VTFunc(f32, body_get_mass, BodyID);
        VTFunc(void, body_set_friction, BodyID, f32);
        VTFunc(f32, body_get_friction, BodyID);
        VTFunc(void, body_apply_force, BodyID, const Vector2&, const Vector2&);
        VTFunc(void, body_apply_impulse, BodyID, const Vector2&, const Vector2&);
        VTFunc(void, body_set_fixed_rotation, BodyID, bool);
        VTFunc(bool, body_is_on_floor, BodyID);
        VTFunc(void, body_set_residence_mask, BodyID, CollisionMask);
        VTFunc(CollisionMask, body_get_residence_mask, BodyID);
        VTFunc(void, body_set_collision_mask, BodyID, CollisionMask);
        VTFunc(CollisionMask, body_get_collision_mask, BodyID);
    };

    struct InternalData
    {
        mem::Allocator allocator;
    };

    static inline VTable vtable;
    static inline InternalData data{};

    static void initialize(const mem::Allocator& allocator, DriverType driver);
    VTFuncDefS(shutdown);
    
    VTFuncDefArg1S(step, f32);
    
    VTFuncDefArg1RetS(BodyID, create_body, Body2D*);
    VTFuncDefArg1S(destroy_body, BodyID);

    VTFuncDefArg2S(body_as_box, BodyID, const Vector2&);
    VTFuncDefArg2S(body_set_type, BodyID, BodyType);
    VTFuncDefArg2S(body_set_velocity, BodyID, const Vector2&);
    VTFuncDefArg1RetS(Vector2, body_get_velocity, BodyID);
    VTFuncDefArg2S(body_set_mass, BodyID, f32);
    VTFuncDefArg1RetS(f32, body_get_mass, BodyID);
    VTFuncDefArg2S(body_set_friction, BodyID, f32);
    VTFuncDefArg1RetS(f32, body_get_friction, BodyID);
    VTFuncDefArg3S(body_apply_force, BodyID, const Vector2&, const Vector2&);
    VTFuncDefArg3S(body_apply_impulse, BodyID, const Vector2&, const Vector2&);
    VTFuncDefArg2S(body_set_fixed_rotation, BodyID, bool);
    VTFuncDefArg1RetS(bool, body_is_on_floor, BodyID);
    VTFuncDefArg2S(body_set_residence_mask, BodyID, CollisionMask);
    VTFuncDefArg1RetS(CollisionMask, body_get_residence_mask, BodyID);
    VTFuncDefArg2S(body_set_collision_mask, BodyID, CollisionMask);
    VTFuncDefArg1RetS(CollisionMask, body_get_collision_mask, BodyID);
};
