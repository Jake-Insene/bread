#include "platform/android/android_display.h"

#include "platform/android/android_engine.h"


void Display::initialize(const mem::Allocator &allocator)
{
    Display::data.allocator = allocator;

    AndroidDisplay::data.window_size = Engine::get_configuration().viewport_size;
    AndroidDisplay::data.window_surface = Graphics::surface_create(
        {
            .window_native_handle = Display::window_get_native_handle(WindowID()),
        }
    );
}

void Display::shutdown()
{
    Graphics::surface_destroy(AndroidDisplay::data.window_surface);
}

Display::WindowID Display::window_create()
{
    return Display::WindowID::invalid();
}

void Display::window_destroy(WindowID window_id)
{
    Unused(window_id);
}

Vector2I Display::window_get_size(Display::WindowID window_id)
{
    Unused(window_id);
    return AndroidDisplay::data.window_size;
}

void Display::window_set_size(Display::WindowID window_id, const Vector2I& new_size)
{
    Unused(window_id, new_size);
}

MemoryAddress Display::window_get_native_handle(Display::WindowID window_id)
{
    Unused(window_id);
    return reinterpret_cast<MemoryAddress>(AndroidEngine::data.app->window);
}

Graphics::SurfaceID Display::window_get_surface(WindowID window_id)
{
    Unused(window_id);
    return AndroidDisplay::data.window_surface;
}

void AndroidDisplay::update_native_size(const Vector2I& new_size)
{
    AndroidDisplay::data.window_size = new_size;
}
