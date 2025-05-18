#pragma once
#include "display/window.h"
#include "input/input.h"
#include "io/texture.h"
#include "mem/generic_allocator.h"


#define ENGINE_CONFIGURATION(...) EngineConfiguration __configuration = {__VA_ARGS__};
#define ENGINE_DEFAULT_CONFIGURATION(main_scene) \
    EngineConfiguration __configuration =\
    {\
        .WindowSize = Vector2I(Display::DefaultWidth, Display::DefaultHeight),\
        .DisplayTargetSize = Vector2I(Display::DefaultWidth, Display::DefaultHeight),\
        .VSync = true,\
        .CreateMainScene = []() -> Object* { return CreateObject<main_scene>(); },\
    }

struct Object;

struct EngineConfiguration
{
    Vector2I WindowSize;
    Vector2I DisplayTargetSize;
    bool VSync;
    Object* (*CreateMainScene)();
};

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
