#include "canvas/canvas_object.h"

#include "2d/object_2d.h"
#include "input/input.h"
#include "render/render_manager.h"
#include "scene/scene_manager.h"


void CanvasObject::_bind_vtable(CanvasObject::VTable& vtable)
{
    BindVTable(vtable, gui_event, &CanvasObject::gui_event);
    BindVTable(vtable, is_inside, &CanvasObject::is_inside);
    BindVTable(vtable, get_rect, &CanvasObject::get_rect);
};

void CanvasObject::init(const CreateInfo&)
{
    data.render_item = RenderManager::create_item();
}

void CanvasObject::deinit()
{
    RenderManager::destroy_item(data.render_item);
}

void CanvasObject::enter()
{
    //if (get_parent() && !get_parent()->has_mark(MARK_CANVAS))
    {
        SceneManager::_add_root_canvas(this);
    }

    CanvasObject* parent_canvas = cast<CanvasObject>(get_parent());
    if (parent_canvas)
    {

        RenderManager::item_set_parent(
            get_render_item(), parent_canvas->get_render_item()
        );
    }

    RenderManager::item_set_layers(get_render_item(), RenderManager::RENDER_LAYER_1);
}

void CanvasObject::exit()
{}

void CanvasObject::gui_event(const InputEvent&) {}

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

    switch (data.render_mode)
    {
    case CANVAS:
        if (parent && parent->has_mark(MARK_CANVAS))
        {
            CanvasObject* p_canvas = reinterpret_cast<CanvasObject*>(parent);
            return p_canvas->get_global_transform() * data.transform;
        }
        break;
    case WORLD:
        if (parent && parent->has_mark(MARK_2D))
        {
            Object2D* p_2d = reinterpret_cast<Object2D*>(parent);
            return p_2d->get_global_transform() * data.transform;
        }
        break;
    }

    return data.transform;
}

void CanvasObject::set_render_mode(RenderMode new_rm)
{
    if (data.render_mode == new_rm)
        return;

    data.render_mode = new_rm;
}

CanvasObject::RenderMode CanvasObject::get_render_mode()
{
    return data.render_mode;
}


bool CanvasObject::is_inside(const Vector2&) const
{
    return false;
}


Rect2D CanvasObject::get_rect() const
{
    return Rect2D();
}

void CanvasObject::draw_canvas_element(const Transform2D& transform, Graphics::TextureID texture, const Rect2D& rect, 
    const Rect2D& src_rect, Color mod_color, u32 flags)
{
    switch (data.render_mode)
    {
    case CANVAS:
    {
        RenderManager::render_item_draw_ui_sprite(
            get_render_item(), transform, texture, rect,
            src_rect, mod_color, RenderManager::RenderFlags(flags)
        );
    }
        break;
    case WORLD:
    {
        RenderManager::render_item_draw_sprite(
            get_render_item(), transform, texture, rect,
            src_rect, mod_color, RenderManager::RenderFlags(flags)
        );
    }
        break;
    }
}
