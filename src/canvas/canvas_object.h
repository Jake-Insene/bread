#pragma once
#include "graphics/graphics_types.h"
#include "math/rect_2d.h"
#include "math/transform_2d.h"
#include "math/color.h"
#include "object/object.h"


/*
* Represent a object in a canvas, useful for UI.
* It is not affected by 2D objects or 2D cameras.
*/
struct CanvasObject : Object
{
    OBJECT(CanvasObject, Object);
    DefineVTable(Object)
    {
        Event<void(CanvasObject::*)(const InputEvent&), false> gui_event;
        Event<bool(CanvasObject::*)(const Vector2&) const, false> is_inside;
        Event<Rect2D(CanvasObject::*)() const, false> get_rect;
    };
    
    static void _bind_vtable(CanvasObject::VTable& vtable);
    
    struct InternalData
    {
        RenderItemID render_item;
        Transform2D transform{};
        Vector2 pos_cache = Vector2(0, 0);
        Vector2 scale_cache = Vector2(1, 1);
        // In Radians
        f32 rot_cache = 0.f;

        Vector2 size_cache;
    } data;
    
    void init(const CreateInfo&);

    void enter();
    void exit();

    /*
    * Handle GUI input, it is activated by the SceneManager when a input event interacts with the obejct.
    * 
    * @param event Contains information about the input that triggers the call.
    */
    void gui_event(const InputEvent& event) Function(FunctionNormal);
    
    RenderItemID get_render_item() { return data.render_item; }

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
    * @param pos A world position vector.
    * 
    * @return True is the given position is inside of the CanvasObject.
    */
    bool is_inside(const Vector2& point) const Function(FunctionNormal);

    /*
    * @return A Rect that cantins all the object and its children.
    */
    Rect2D get_rect() const;

    void draw_canvas_element(const Transform2D& transform, TextureID texture, const Rect2D& rect,
        const Rect2D& src_rect, Color mod_color, u32 flags);
};

