#include "input/input.h"



bool Input::is_key_down(Key k)
{
	return data.keys[(int)k] == KeyState::Pressed;
}

KeyState Input::get_key_state(Key k)
{
	return data.keys[(int)k];
}


bool Input::is_mouse_button_pressed(MouseButton button)
{
	return data.mouse_buttons[button];
}

bool Input::is_mouse_button_just_pressed(MouseButton button)
{
	bool pressed = data.mouse_buttons[button];
	data.mouse_buttons[button] = false;
	return pressed;
}

Vector2 Input::get_mouse_position()
{
	return data.mouse_position;
}
