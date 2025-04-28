#include "gui/button.h"

#include "graphics/graphics.h"
#include "io/resource_manager.h"
#include "math/rect_2d.h"


void Button::_bind_vtable(VTable& vtable)
{
    vtable.is_inside = (bool(CanvasObject::*)(Vector2) const)&Button::is_inside;
}

void Button::init(const CreateInfo&)
{
    normal_texture = GetResource<Texture2D>("white.png");
    pressed_texture = GetResource<Texture2D>("white.png");
    hover_texture = GetResource<Texture2D>("white.png");
    
    current_texture = normal_texture;
    
    data.color = {255, 255, 255, 255};
}

void Button::render()
{
    Vector2 texsize = Vector2(current_texture->get_size());
    Graphics::add_cmd(
        RenderCommand
        {
            .type = RenderCommand::DRAW_SPRITE,
            .sprite =
            {
                .transform = data.transform,
                .texture_extent = texsize,
                .src_rect = Rect2D(Vector2(), texsize),
                .texture = current_texture->texture_id,
                .color = data.color,
                .flags = RenderCommand::FLAG_TOP_LEFT,
            }
        }
    );
}

void Button::event(const InputEvent& e)
{
    if(e.type == INPUT_EVENT_TOUCH)
    {
        const auto& et = e.get<InputEventTouch>();
        if(is_inside(et.position))
        {
            current_texture = pressed_texture;
            is_pressed = et.pressed;
        }
        else
        {
            current_texture = normal_texture;
            is_pressed = false;
        }
    }
    
	if (e.type == INPUT_EVENT_MOUSE_BUTTON)
	{
		const auto& em = e.get<InputEventMouseButton>();
		if (is_inside(em.position))
		{
			current_texture = hover_texture;
			is_pressed = em.left;
		}
        else
        {
            current_texture = normal_texture;
            is_pressed = false;
        }
	}
}

bool Button::is_inside(Vector2 pos) const
{
    return Rect2D(
            get_position(),
            Vector2(normal_texture->get_size()) * get_scale()
    ).is_in_area(pos);
}
