#include "platform/android/android_display.h"

#include "platform/android/android_engine.h"


void Display::initialize(mem::Allocator* allocator)
{
    // Ensures constructors are call.
    ConstructObject(AndroidDisplay::data);

    AndroidDisplay::data.allocator = allocator;

    AndroidDisplay::update_native_size(
            Vector2I(
                    ANativeWindow_getWidth(AndroidEngine::data.app->window),
                    ANativeWindow_getHeight(AndroidEngine::data.app->window)
            )
    );
    AndroidDisplay::data.window_surface = GPU::surface_create(
        {
            .window_native_handle = Display::window_get_native_handle(WindowID()),
        }
    );
}

void Display::shutdown()
{
    GPU::surface_destroy(AndroidDisplay::data.window_surface);
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

GPU::SurfaceID Display::window_get_surface(WindowID window_id)
{
    Unused(window_id);
    return AndroidDisplay::data.window_surface;
}

void AndroidDisplay::update_native_size(const Vector2I& new_size)
{
    AndroidDisplay::data.window_size = new_size;
}
