#include "display/window.h"



Display::WindowID Window::create()
{
    return Display::window_create();
}

void Window::set_size(const Vector2I& new_size) const
{
    Display::window_set_size(window_id, new_size);
}

Vector2I Window::get_size() const
{
    return Display::window_get_size(window_id);
}

MemoryAddress Window::get_native_handle() const
{
    return Display::window_get_native_handle(window_id);
}
