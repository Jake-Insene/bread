#include "audio/audio.h"
#include "display/display.h"
#include "Debug/Fail.hpp"
#include "engine/application.h"
#include "gpu/gpu.h"
#include "Mem/GenericAllocator.hpp"
#include "os/os.h"


void* operator new(size_t)
{
    FailOn(true, "avoid 'new' statements!");
    return reinterpret_cast<void*>(0xFFFFFFFF'FFFFFFFF);
}

void* operator new[](size_t)
{
    FailOn(true, "avoid 'new' statements!");
    return reinterpret_cast<void*>(0xFFFFFFFF'FFFFFFFF);
}

void operator delete(void*)
{
    FailOn(true, "avoid 'delete' statements!");
}

void operator delete[](void*)
{
    FailOn(true, "avoid 'delete' statements!");
}

extern ApplicationInfo RegisterApplication();

bool Platform_Poll();
Application* app;

void app_loop(Mem::Allocator& allocator)
{
    ApplicationInfo app_info = RegisterApplication();

    app = reinterpret_cast<Application*>(
        allocator.alloc(app_info.size_in_bytes, app_info.alignment).ptr());

    app_info.constructor(Core::Opaque::from(*app), {.allocator = allocator});

    app->initialize({.allocator = allocator});

    bool quit = false;
    while(!quit)
    {
        app->pre_tick();

        if(Platform_Poll())
        {
            quit = true;
        }
        else
        {
            app->tick();
        }
    }

    app->shutdown();
    Core::Mem::Destruct(*app);
    allocator.free(Mem::to_bytes(Slice(app, 1)));
}

void bread_main()
{
    Mem::GenericAllocator global_allocator;

    Log::debug("[Runtime]: Initializing...");

    OS::initialize(global_allocator);
    Display::initialize(global_allocator);
    Audio::initialize(global_allocator, Audio::DriverType::Default);
    GPU::initialize(global_allocator);

    app_loop(global_allocator);

    GPU::shutdown();
    Audio::shutdown();
    Display::shutdown();
    OS::shutdown();
}

void bread_handle_event(const Event& event)
{
    app->handle_event(event);
}

