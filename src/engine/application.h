#pragma once
#include "core/header.h"
#include "math/vec2.h"
#include "math/vec2.h"


struct Event;

namespace mem
{
struct Allocator;
}

struct ApplicationInfo
{
    usize size_in_bytes;
    usize alignment;
    void(*constructor)(Opaque*);
    Vector2I viewport_size;
    bool vsync;
    bool keep_viewport;
    bool enable_debug_console;
};

struct Application
{
    Application();
    virtual ~Application();

    virtual void initialize(mem::Allocator* allocator);
    virtual void shutdown();

    virtual void load_resources();
    virtual void unload_resources();

    virtual void update(f32 dt);
    virtual void render();

    virtual void event(const Event& e);
};
