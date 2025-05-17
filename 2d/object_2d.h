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
        Vector2 pos_cache{};
        Vector2 scale_cache{ 1, 1 };
        // In defress
        f32 rot_cache{};
    } data;
    
    void init(const CreateInfo& info);
    
    // 2D utility
    void set_position(Vector2 npos);
    Vector2 get_position() const;
    void translate(Vector2 t);
    
    void set_scale(Vector2 nscale);
    Vector2 get_scale() const;
    
    void set_rotation(f32 nrot);
    [[nodiscard]] f32 get_rotation() const;

    [[nodiscard]] Transform2D get_transform() const;
};
