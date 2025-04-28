#pragma once
#include "mem/allocator.h"
#include "math/vec2.h"

struct EGL
{
    struct InternalData
    {
        mem::Allocator allocator;
        
        Vector2I surface_size;
    };
    
    static inline InternalData data;
    
    static void initialize(mem::Allocator& allocator);
    static void shutdown();
    
    static void recreate_window_surface();
    static void uncreate_window_surface();
    
    static void present();
    
    static Vector2I get_surface_size();
};
