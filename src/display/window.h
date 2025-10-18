#pragma once
#include "display/display.h"

using WindowID = Display::WindowID;


struct Window
{
    WindowID window_id;
    
    static WindowID create();

    void set_size(const Vector2I& new_size) const;
    Vector2I get_size() const;
    Opaque get_native_handle() const;
};
