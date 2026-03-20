#pragma once
#include "math/vec2.h"
#include "display/display.h"
#include "systems/system.h"


struct Scene;

// Application configuration
#define EngineConfiguration(...) EngineConfiguration __configuration__ = {__VA_ARGS__};
#define DefaultCreateScene(name) [](const mem::Allocator& allocator) -> Scene* { return Scene::create<name>(allocator); }
#define EngineDefaultConfiguration(main_scene) \
    EngineConfiguration __configuration__ =\
    {\
        .create_main_scene = DefaultCreateScene(main_scene),\
    }

struct EngineConfiguration
{
    Vector2I viewport_size = Vector2I(Display::DefaultWidth, Display::DefaultHeight);
    Scene* (*create_main_scene)(const mem::Allocator&);
    bool vsync;
    bool keep_viewport;
    bool enable_debug_console;
    bool enable_custom_rendering;
    Slice<SystemInfo> requested_systems;
};