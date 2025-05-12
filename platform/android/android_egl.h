#pragma once
#include "graphics/egl/egl.h"
#include "platform/platform_header.h"

struct AndroidEGL
{
    struct InternalData
    {
        EGLConfig config;
        EGLSurface surface;
        EGLContext context;
        EGLDisplay display;
    };
    
    static inline InternalData data;
    
    static EGL::VTable get_vtable();

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();
    
    static void recreate_window_surface();
    static void destroy_window_surface();
    
    static void present();
};
