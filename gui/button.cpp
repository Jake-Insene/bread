#include "gui/button.h"

#include "graphics/graphics.h"
#include "input/input.h"
#include "math/rect_2d.h"
#include "resource/resource_manager.h"


void Button::_bind_vtable(VTable& vtable)
{
    vtable.is_inside.bind(&Button::is_inside);
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
    Vector2 texture_extent = Vector2(current_texture->get_size());
    Graphics::add_cmd(
        RenderCommand
        {
            .type = RenderCommand::DRAW_SPRITE,
            .sprite =
            {
                .transform = data.transform,
                .texture_extent = texture_extent,
                .dest_extent = size,
                .src_rect = Rect2D(Vector2(), texture_extent),
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
    
	else if (e.type == INPUT_EVENT_MOUSE_BUTTON)
	{
		const auto& em = e.get<InputEventMouseButton>();
        if(em.button == MOUSE_BUTTON_LEFT && em.pressed)
        {
            is_pressed = is_inside(em.position);

            if (is_pressed)
                current_texture = pressed_texture;
            else
                current_texture = normal_texture;
        }
        else
        {
            is_pressed = false;
            current_texture = normal_texture;
        }
	}
}

bool Button::is_inside(const Vector2& pos) const
{
    return Rect2D(
            get_position(),
            size * get_scale()
    ).point_is_in(pos);
}
