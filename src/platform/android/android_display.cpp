#include "platform/android/android_display.h"

#include "platform/android/android_engine.h"


void Display::initialize(const mem::Allocator &allocator)
{
    Display::data.allocator = allocator;
}

void Display::shutdown()
{}

Display::WindowID Display::window_create()
{
    return Display::WindowID();
}

Vector2I Display::window_get_size(Display::WindowID)
{
    return AndroidDisplay::data.window_size;
}

void Display::window_set_size(Display::WindowID, const Vector2I&)
{
}

MemoryAddress Display::window_get_native_handle(Display::WindowID)
{
    return reinterpret_cast<MemoryAddress>(AndroidEngine::data.app->window);
}

void AndroidDisplay::update_native_size(Vector2I new_size)
{
    AndroidDisplay::data.window_size = new_size;
}
