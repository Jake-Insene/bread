#pragma once
#include "display/display.h"


struct Window
{
    Display::WindowID window_id;
    
    static Display::WindowID create();

    void destroy() const;

    void set_size(const Vector2I& new_size) const;
    Vector2I get_size() const;
    MemoryAddress get_native_handle() const;
    Graphics::SurfaceID get_surface() const;
};
