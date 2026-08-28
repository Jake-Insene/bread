#pragma once
#include "concurrency/job_queue.h"
#include "display/display.h"
#include "display/window.h"
#include "Math/vec2.h"


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
    void(*constructor)(Core::Opaque*, const ApplicationAllocateInfo&);
};

struct ApplicationAllocateInfo
{
    Mem::Allocator& allocator;
};

struct ApplicationInitializeInfo
{
    Mem::Allocator& allocator;
};

struct Application
{
    static constexpr usize DefaultMainQueueSize = 16;

    struct InternalData
    {
        Mem::Allocator& allocator;
        Core::Version runtime_version;
        ApplicationInfo application_info;

        GPU::PhysicalDeviceID selected_physical_device;

        JobQueue main_queue;

        Window main_window;

        struct
        {
            f64 update_time = 0;
            f64 render_time = 0;
        } debug_time;

        f32 last_time = 0;
        f32 time_accum = 0;

        i32 fps = 0;
        i32 fps_counter = 0;
        i32 fps_accum = 0;
        f32 delta_time = 0;

        bool can_tick = false;

        InternalData(Mem::Allocator& allocator)
        : allocator(allocator), runtime_version(Core::RuntimeVersion),
        main_queue(allocator, DefaultMainQueueSize)
        {}
    } data;

    Window window;

    Application(const ApplicationAllocateInfo& alloc_info);
    virtual ~Application();

    virtual void initialize(const ApplicationInitializeInfo& init_info);
    virtual void shutdown();

    virtual void update(f32 dt);
    virtual void render();

    virtual void event(const Event& e);

    void handle_event(const Event& e);
    void pre_tick();
    void tick();

    void run();
};
