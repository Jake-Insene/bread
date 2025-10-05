#pragma once
#include "graphics/graphics_types.h"
#include "object/object.h"
#include "math/color.h"
#include "math/transform_2d.h"
#include "math/rect_2d.h"


/*
* Represent a object in a 2D world.
* It is affected by 2D cameras.
*/
struct Object2D : Object
{
    OBJECT(Object2D, Object);
    
    struct InternalData
    {
        RenderItemID render_item;
        Transform2D transform{};
        Transform2D global_transform_cache{};
        Vector2 pos_cache = Vector2(0, 0);
        Vector2 scale_cache = Vector2(1, 1);
        // In Radians
        f32 rot_cache = 0.f;
    } data;
    
    void init(const CreateInfo& info);
    void enter();
    void exit();
    
    RenderItemID get_render_item() { return data.render_item; }

    void set_position(Vector2 new_pos);
    Vector2 get_position() const;

    void translate(Vector2 t);
    
    void set_scale(Vector2 new_scale);
    Vector2 get_scale() const;
    
    void set_rotation(f32 new_rot);
    [[nodiscard]] f32 get_rotation() const;

    void rotate(const f32 rads);

    Transform2D get_transform() const;
    Transform2D get_global_transform() const;

    Vector2 get_local_mouse_position() const;

    void draw_sprite(const Transform2D& transform, TextureID texture, const Rect2D& rect,
        const Rect2D& src_rect, Color mod_color, u32 flags);

    void _update_transform();
    Transform2D _make_global_transform() const;
};
