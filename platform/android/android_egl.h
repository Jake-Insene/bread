#pragma once
#include "graphics/egl/egl.h"
#include "platform/platform_header.h"

struct AndroidEGL : EGL
{
    struct InternalData
    {
        EGLConfig config;
        EGLSurface surface;
        EGLContext context;
        EGLDisplay display;
    };
    
    static inline InternalData data;
    
    static void initialize(mem::Allocator& allocator);
    static void shutdown();
    
    static void recreate_window_surface();
    static void destroy_window_surface();
    
    static void present();
};
