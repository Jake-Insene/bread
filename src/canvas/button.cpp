#include "canvas/button.h"

#include "graphics/render_manager.h"
#include "input/input.h"
#include "math/rect_2d.h"
#include "resource/texture.h"


void Button::_bind_vtable(VTable& vtable)
{
    BindVTable(vtable, is_inside, &Button::is_inside);
    BindVTable(vtable, gui_event, &Button::gui_event);
}

void Button::init(const CreateInfo&)
{
    normal_texture = Resource::load<Texture2D>("default/white.png");
    
    data.current_state = STATE_NORMAL;
}

void Button::render()
{
    Texture* current_texture = get_current_texture();
    if (current_texture == nullptr && normal_texture != nullptr)
        current_texture = normal_texture;

    if (current_texture == nullptr)
        return;

    Vector2 size = get_size();
    Vector2 texture_extent = Vector2(current_texture->get_size());
    draw_canvas_element(
        get_global_transform(), current_texture->texture_id,
        Rect2D(Vector2(), size),
        Rect2D(Vector2(), texture_extent),
        Color(255, 255, 255, 255), 
        RenderManager::RENDER_FLAG_NONE
    );
}

void Button::gui_event(const InputEvent& e)
{
    if(e.type == INPUT_EVENT_TOUCH)
    {
        const auto& et = e.get<InputEventTouch>();
        data.current_state = is_inside(et.position) ? STATE_PRESSED : STATE_NORMAL;
    }
	else if (e.type == INPUT_EVENT_MOUSE_BUTTON)
	{
		const auto& em = e.get<InputEventMouseButton>();
        if(em.button == MouseButton::Left && em.pressed)
        {
            data.current_state = is_inside(em.position) ? STATE_PRESSED : STATE_NORMAL;
        }
        else
        {
            data.current_state = STATE_NORMAL;
        }
	}

    if (on_pressed.has_func() && data.current_state == STATE_PRESSED)
    {
        on_pressed.call();
    }
    else if (on_released.has_func() && data.current_state == STATE_NORMAL)
    {
        on_released.call();
    }
}

bool Button::is_inside(const Vector2& point) const
{
    Transform2D global_transform = get_global_transform();
    return Rect2D(
            global_transform.get_position(),
            get_size() * global_transform.get_scale()
    ).point_is_in(point);
}

Rect2D Button::get_rect() const
{
    return Rect2D(get_position(), get_scale() * get_size());
}

Texture* Button::get_current_texture() const
{
    switch (data.current_state)
    {
    case STATE_NORMAL:
        return normal_texture;
    case STATE_PRESSED:
        return pressed_texture;
    case STATE_HOVERED:
        return hover_texture;
    }

    return nullptr;
}
