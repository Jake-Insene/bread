#pragma once
#include "mem/allocator.h"
#include "graphics/graphics_types.h"
#include "graphics/render_target.h"
#include "math/color.h"
#include "math/transform_2d.h"
#include "math/rect_2d.h"


/*
* Contains a world to render to.
*/
struct [[nodiscard]] Viewport
{
    RenderTarget rt;
    Transform2D scene_transform;

    Vector2I viewport_size;
    Color clear_color;
    bool must_sync;

    static Viewport create_from_render_target(const mem::Allocator& allocator, RenderTarget rt_id);
    void destroy();

    void set_scene_transform(const Transform2D& transform);
    Transform2D get_scene_transform() const { return scene_transform; }

    void set_size(const Vector2I& new_size);
    Vector2I get_size() const;

    Vector2 get_local_mouse_position() const;
};