#include "Runtime/Application.hpp"

#include "Debug/Log.hpp"
#include "Debug/Profiler.hpp"
#include "input/input.h"
#include "os/os.h"



Application::Application(const ApplicationAllocateInfo& alloc_info)
: data(alloc_info.allocator), window(Window::create())
{
    data.last_time = f32(OS::get_time());
    data.can_tick = true;
}

Application::~Application()
{
    window.destroy();
}

void Application::initialize(const ApplicationInitializeInfo&)
{
}

void Application::shutdown()
{
}

void Application::update(f32)
{
}

void Application::render()
{
}

void Application::event(const Event&) {}

void Application::handle_event(const Event& e)
{
    if(e.type == EventType::WindowClose)
    {
        const InputEventWindowClose& close_event = e.get<InputEventWindowClose>();
        if(close_event.window == window.window_id)
        {
            data.can_tick = false;
        }
    }
    
    if(data.can_tick)
    {
        event(e);
    }
}

void Application::pre_tick()
{
    Input::update();
}

void Application::tick()
{
    if(!data.can_tick)
    {
        return;
    }

    f32 current = f32(OS::get_time());
    data.delta_time = current - data.last_time;
    data.last_time = current;

    data.time_accum += data.delta_time;
    if (data.time_accum >= 1.0)
    {
        data.fps_counter = data.fps_accum;
        data.fps = data.fps_counter;
        Log::info(
            "Frame Info: FPS: {}\n"
            "\tAvg Frame Time: {}\n"
            "\tUpdate Time: {}\n"
            "\tRender Time: {}\n",
            data.fps_counter, data.delta_time,
            data.debug_time.update_time, 
            data.debug_time.render_time
        );

        data.fps_accum = 0;
        data.time_accum = 0;
    }

    {
        PROFILE_SCOPE(
            data.debug_time.update_time = duration;
        );

        update(data.delta_time);
    }

    {
        PROFILE_SCOPE(
            data.debug_time.render_time = duration;
        );

        render();
        data.fps_accum++;
    }

    data.main_queue.run();
}

bool Platform_Poll();

void Application::run()
{
    initialize({.allocator = data.allocator});

    bool quit = false;
    while(!quit)
    {
        pre_tick();

        if(Platform_Poll())
        {
            quit = true;
        }
        else
        {
            tick();
        }
    }

    shutdown();
}
