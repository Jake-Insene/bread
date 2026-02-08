#pragma once
#include "collections/free_list.h"
#include "display/display.h"
#include "platform/platform_header.h"


struct Win32Display
{
    static constexpr const char* WindowClassName = "Bread:window";
    
    struct WindowData
    {
        Display::WindowID self_id;
        HWND handle;
        RECT window_rect;
        bool mouse_captured;
    };

    struct InternalData
    {
        mem::Allocator allocator;

        FreeList<WindowData, Display::WindowID> windows;
        RECT fullscreen_rect;
    };

    static inline InternalData data;
};

