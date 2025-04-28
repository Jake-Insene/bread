#pragma once
#include "io/input.h"
#include "io/texture.h"
#include "mem/generic_allocator.h"

struct Engine
{
    struct InternalData
    {
        mem::GenericAllocator allocator;
        
        Texture* white_texture;

        i32 fps = 0;
    };
    
    static inline InternalData data;
    
    static void initialize();
    static void shutdown();
    
    static void recreate_window();
    static void destroy();
    
    static void step();
    static void handle_input(const InputEvent& event);

    static i32 get_fps() { return data.fps; }
};
