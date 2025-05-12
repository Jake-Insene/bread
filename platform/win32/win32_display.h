#pragma once
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

    static Display::VTable get_vtable();

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

    static Display::WindowID window_create();
    static Vector2I window_get_size(Display::WindowID wid);
    static void window_set_size(Display::WindowID wid, const Vector2I& new_size);
    static void* window_get_native_handle(Display::WindowID wid);
};

