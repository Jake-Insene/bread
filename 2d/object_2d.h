#pragma once
#include "object/object.h"

#include "math/transform_2d.h"

struct Object2D : Object
{
    OBJECT(Object2D, Object);

    static void _bind_vtable(VTable& vtable);

    // As everything in a struct is public we need to hide data
    // that should not be modified/access directly, this also
    // resolve some namespace problems.
    struct InternalData
    {
        Transform2D transform{};
        Vector2 pos_cache;
        Vector2 scale_cache = Vector2(1, 1);
        // In Radians
        f32 rot_cache{};
    } data;
    
    void init(const CreateInfo& info);
    
    // 2D utility
    void set_position(Vector2 new_pos);
    Vector2 get_position() const;
    void translate(Vector2 t);
    
    void set_scale(Vector2 new_scale);
    Vector2 get_scale() const;
    
    void set_rotation(f32 new_rot);
    [[nodiscard]] f32 get_rotation() const;

    [[nodiscard]] Transform2D get_transform() const;
    [[nodiscard]] Transform2D get_global_transform() const;
};
