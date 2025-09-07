#include "2d/object_2d.h"

#include "input/input.h"
#include "scene/scene_manager.h"
#include "2d/camera_2d.h"


void Object2D::init(const CreateInfo&)
{
    mark(MARK_2D);
}

void Object2D::enter()
{
    data.render_item = get_viewport()->create_item(Viewport::VIEWPORT_LAYER_DEFAULT);
}

void Object2D::exit()
{
    get_viewport()->destroy_item(data.render_item);
}

void Object2D::set_position(Vector2 new_pos)
{
    data.pos_cache = new_pos;
    data.transform.set_position(new_pos);
}

Vector2 Object2D::get_position() const
{
    return data.pos_cache;
}

void Object2D::translate(Vector2 t)
{
    data.pos_cache += t;
    data.transform.translate(t);
}

void Object2D::set_scale(Vector2 new_scale)
{
    if (data.scale_cache == new_scale)
        return;

    data.scale_cache = new_scale;
    data.transform.set_scale(new_scale);
}

Vector2 Object2D::get_scale() const
{
    return data.scale_cache;
}

void Object2D::set_rotation(f32 new_rot)
{
    if (data.rot_cache == new_rot)
        return;
 
    data.rot_cache = new_rot;
    data.transform.set_rotation(new_rot);
}

f32 Object2D::get_rotation() const
{
    return data.rot_cache;
}

Transform2D Object2D::get_transform() const
{
    return data.transform;
}

Transform2D Object2D::get_global_transform() const
{
    Object* parent = get_parent();
    if (parent && parent->has_mark(MARK_2D))
    {
        Object2D* p2d = (Object2D*)parent;
        return p2d->get_global_transform() * data.transform;
    }

    return data.transform;
}

Vector2 Object2D::get_local_mouse_position() const
{
    const Camera2D* cam = SceneManager::get_camera_2d();
    const Vector2 display_size = Vector2(SceneManager::get_viewport_size());
    const Vector2 screen_pos = Input::get_mouse_position();
    const Vector2 local_pos = screen_pos - Vector2(display_size.x, -display_size.y) * 0.5f;

    if (cam)
    {
        return cam->get_global_transform() * local_pos;
    }

    return local_pos;
}

void Object2D::draw_sprite(const Transform2D& transform, TextureID texture, const Rect2D& rect, 
    const Rect2D& src_rect, Color mod_color, u32 flags)
{
    get_viewport()->render_item_draw_sprite(
        get_render_item(), transform, texture, rect, src_rect, 
        mod_color, Viewport::RenderFlags(flags)
    );
}
