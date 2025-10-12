#include "canvas/canvas_object.h"

#include "scene/scene_manager.h"


void CanvasObject::_bind_vtable(CanvasObject::VTable& vtable)
{
    BindVTable(vtable, gui_event, &CanvasObject::gui_event);
    BindVTable(vtable, is_inside, &CanvasObject::is_inside);
    BindVTable(vtable, get_rect, &CanvasObject::get_rect);
};

void CanvasObject::init(const CreateInfo&)
{
    mark(MARK_RENDER);
    mark(MARK_CANVAS);
}

void CanvasObject::enter()
{
    //if (get_parent() && !get_parent()->has_mark(MARK_CANVAS))
    {
        SceneManager::_add_root_canvas(this);
    }

    data.render_item = get_viewport()->create_item(Viewport::VIEWPORT_LAYER_DEFAULT);
}

void CanvasObject::exit()
{
    get_viewport()->destroy_item(data.render_item);
}

void CanvasObject::set_position(Vector2 new_pos)
{
    data.transform.set_position(new_pos);
    data.pos_cache = new_pos;
}

Vector2 CanvasObject::get_position() const
{
    return data.pos_cache;
}

void CanvasObject::set_size(Vector2 new_size)
{
    data.size_cache = new_size;
}

Vector2 CanvasObject::get_size() const
{
    return data.size_cache;
}

void CanvasObject::set_scale(Vector2 new_scale)
{
    data.transform.set_scale(new_scale);
    data.scale_cache = new_scale;
}

Vector2 CanvasObject::get_scale() const
{
    return data.scale_cache;
}

void CanvasObject::set_rotation(f32 new_rot)
{
    data.transform.set_rotation(new_rot);
    data.rot_cache = new_rot;
}

f32 CanvasObject::get_rotation() const
{
    return data.rot_cache;
}

Transform2D CanvasObject::get_transform() const
{
    return data.transform;
}

Transform2D CanvasObject::get_global_transform() const
{
    Object* parent = get_parent();
    if (parent && parent->has_mark(MARK_CANVAS))
    {
        CanvasObject* p_canvas = (CanvasObject*)parent;
        return p_canvas->get_global_transform() * data.transform;
    }

    return data.transform;
}


bool CanvasObject::is_inside(const Vector2&) const
{
    return false;
}


Rect2D CanvasObject::get_rect() const
{
    return Rect2D();
}

void CanvasObject::draw_canvas_element(const Transform2D& transform, TextureID texture, const Rect2D& rect, 
    const Rect2D& src_rect, Color mod_color, u32 flags)
{
    get_viewport()->render_item_draw_ui_sprite(
        get_render_item(), transform, texture, rect,
        src_rect, mod_color, Viewport::RenderFlags(flags)
    );
}
