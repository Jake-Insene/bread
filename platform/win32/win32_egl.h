#pragma once
#include "graphics/egl/egl.h"
#include "platform/platform_header.h"

struct Win32EGL : EGL
{
    struct WGLVTable
    {
        PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
		PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
    };

    struct InternalData
    {
        HWND current_window;
		HDC device_context;
        HGLRC context;
    };

    static inline InternalData data;
    static inline WGLVTable wgl;

    static void initialize(mem::Allocator& allocator);
    static void shutdown();

    static void recreate_window_surface();
    static void destroy_window_surface();

    static void present();
};
