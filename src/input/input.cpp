#include "input/input.h"


void Input::update()
{
	Mem::copy(Slice(data.keys_prev), Slice(data.keys));
	Mem::copy(Slice(data.mouse_buttons_prev), Slice(data.mouse_buttons));
}

void Input::update_key_state(Key k, KeyState new_state)
{
	data.keys[i32(k)] = new_state;
}

void Input::update_mouse_button(MouseButton button, bool state)
{
	data.mouse_buttons[i32(button)] = state;
}

KeyState Input::get_key_state(Key k)
{
	return data.keys[i32(k)];
}

bool Input::get_mouse_state(MouseButton button)
{
	return data.mouse_buttons[i32(button)];
}

bool Input::is_key_down(Key k)
{
	return data.keys[i32(k)] == KeyState::Pressed;
}

bool Input::is_key_just_down(Key k)
{
	return data.keys[i32(k)] == KeyState::Pressed
		&& !(data.keys_prev[i32(k)] == KeyState::Pressed);
}

bool Input::is_mouse_button_pressed(MouseButton button)
{
	return data.mouse_buttons[i32(button)];
}

bool Input::is_mouse_button_just_pressed(MouseButton button)
{
	return data.mouse_buttons[i32(button)]
		&& !data.mouse_buttons_prev[i32(button)];
}

Vector2 Input::get_mouse_position()
{
	return data.mouse_position;
}
