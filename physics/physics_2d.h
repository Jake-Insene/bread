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

    struct VTable
    {
        VTFunc(void, initialize, mem::Allocator&);
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
    };

    struct InternalData
    {
        mem::Allocator allocator;
    };

    static inline VTable vtable;
    static inline InternalData data{};

    static void initialize(mem::Allocator& allocator, DriverType driver);
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
};
