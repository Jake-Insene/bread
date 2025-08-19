#pragma once
#include "object/object.h"
#include "math/rect_2d.h"
#include "math/transform_2d.h"
#include "math/color.h"


/*
* Represent a object in a canvas, useful for UI.
* It is not affected by 2D objects or 2D cameras.
*/
struct CanvasObject : Object
{
    OBJECT(CanvasObject, Object);
    DefineVTable(Object)
    {
        Event<bool(CanvasObject::*)(const Vector2&) const, false> is_inside;
        Event<bool(CanvasObject::*)(const Vector2&) const, false> get_rect;
    };
    
    static void _bind_vtable(CanvasObject::VTable& vtable);
    
    // As everything in a struct is public we need to hide data
    // that should not be modified/access directly, this also
    // resolve some namespace problems.
    struct InternalData
    {
        Transform2D transform{};
        Vector2 pos_cache = Vector2(0, 0);
        Vector2 scale_cache = Vector2(1, 1);
        // In Radians
        f32 rot_cache = 0.f;

        Vector2 size_cache;
    } data;
    
    void init(const CreateInfo&);

    void enter();
    
    void set_position(Vector2 new_pos);
    Vector2 get_position() const;

    void set_size(Vector2 new_size);
    Vector2 get_size() const;

    void set_scale(Vector2 new_scale);
    Vector2 get_scale() const;

    void set_rotation(f32 new_rot);
    f32 get_rotation() const;
    
    Transform2D get_transform() const;
    Transform2D get_global_transform() const;

    /*
    * Return true is the given position is inside of the CanvasObject.
    * @param pos A world position vector.
    */
    bool is_inside(const Vector2& pos) const;

    Rect2D get_rect() const;
};

