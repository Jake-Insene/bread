#include "graphics/viewport.h"

#include "input/input.h"


Viewport Viewport::create_from_render_target(const mem::Allocator&, RenderTarget rt)
{
	Viewport vp = {};
	vp.rt = rt;
	vp.viewport_size = Vector2I();
	vp.clear_color = Color(0, 0, 0, 255);
	vp.must_sync = false;
	return vp;
}

void Viewport::destroy()
{
    rt.destroy();
}

void Viewport::set_scene_transform(const Transform2D& transform)
{
	scene_transform = transform;
}

void Viewport::set_size(const Vector2I& new_size)
{
	viewport_size = new_size;
	must_sync = true;
	rt.set_size(new_size);
}

Vector2I Viewport::get_size() const
{
	return viewport_size;
}

Vector2 Viewport::get_local_mouse_position() const
{
    Vector2 mouse_pos = Input::get_mouse_position();
    // Flip Y to convert from screen coordinates (Y down) to local coordinates (Y up)
    Vector2 flipped_mouse = Vector2(-mouse_pos.x, -mouse_pos.y);
    // Apply inverse scene transform to convert to world/local space
    Transform2D inverse_transform = scene_transform.inverse();
    Vector2 local_pos = inverse_transform * flipped_mouse;
	local_pos.y *= -1;
	local_pos.x *= -1;
    return local_pos;
}

