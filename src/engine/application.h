#pragma once
#include "math/vec2.h"
#include "display/display.h"
#include "graphics/render_device.h"
#include "graphics/swap_chain.h"


struct Event;

namespace Mem
{
struct Allocator;
}

struct ApplicationAllocateInfo;

struct ApplicationInfo
{
    usize size_in_bytes;
    usize alignment;
    void(*constructor)(Opaque*, const ApplicationAllocateInfo&);
    Vector2I viewport_size;
    bool vsync;
    bool keep_viewport;
    bool enable_debug_console;
};

struct ApplicationAllocateInfo
{
    Mem::Allocator* allocator;
    Graphics::RenderDevice* render_device;
    Display::WindowID window;
};

struct ApplicationInitializeInfo
{
    Mem::Allocator* allocator;
};

struct Application
{
    struct InternalData
    {
        Graphics::SwapChain swap_chain;
    } data;

    Application(const ApplicationAllocateInfo& alloc_info);
    virtual ~Application();

    virtual void initialize(const ApplicationInitializeInfo& init_info);
    virtual void shutdown();

    virtual void load_resources();
    virtual void unload_resources();

    virtual void update(f32 dt);
    virtual void render();

    virtual void event(const Event& e);
};
