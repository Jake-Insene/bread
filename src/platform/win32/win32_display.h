#pragma once
#include "collections/queue_array.h"
#include "platform/platform_header.h"
#include "display/display.h"


struct Win32Display
{
    static constexpr const char* WindowClassName = "Bread:window";
    
    struct WindowData
    {
        Display::WindowID self_id;
        HWND handle;
        RECT window_rect;
    };

    struct InternalData
    {
        mem::Allocator allocator;

        QueueArray<WindowData, Display::WindowID> windows;
        RECT fullscreen_rect;
    };

    static inline InternalData data;
};

