#pragma once
#include "object/object.h"
#include "math/transform_2d.h"
#include "math/color.h"


// Used currently in UI stuff, 2D objects are in a 2d world not a canvas.
struct CanvasObject : Object
{
    OBJECT(CanvasObject, Object);
    DefineVTable(Object)
    {
        Event<bool(CanvasObject::*)(const Vector2&) const, false> is_inside;
    };
    
    static void _bind_vtable(CanvasObject::VTable& vtable);
    
    // As everything in a struct is public we need to hide data
    // that should not be modified/access directly, this also
    // resolve some namespace problems.
    struct InternalData
    {
        Transform2D transform;
        
        Color color;
    } data;
    
    void init(const CreateInfo&);
    
    void set_position(Vector2 npos);
    Vector2 get_position() const;
    
    void set_scale(Vector2 nscale);
    Vector2 get_scale() const;
    
    bool is_inside(const Vector2& pos) const;
};

