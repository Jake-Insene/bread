#pragma once
#include "display/window.h"
#include "io/input.h"
#include "io/texture.h"
#include "mem/generic_allocator.h"

struct Engine
{
    struct VTable
    {
    };

    struct InternalData
    {
        mem::GenericAllocator allocator;
        
        Window main_window;

        Texture* white_texture;

        i32 fps;
    };

    static inline VTable vtable;
    
    static inline InternalData data;
    
    static void initialize();
    static void shutdown();
    
    static void recreate_window();
    static void destroy();
    
    static void step();
    static void handle_input(const InputEvent& event);

    // Utility functions
    static i32 get_fps() { return data.fps; }

    static Window get_main_window() { return data.main_window; }

    static void set_vsync(bool vsync);
};
