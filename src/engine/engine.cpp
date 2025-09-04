#include "engine/engine.h"

#include "audio/audio.h"
#include "display/display.h"
#include "graphics/graphics.h"
#include "graphics/egl/egl.h"
#include "log/log.h"
#include "object/object_allocator.h"
#include "scene/scene_manager.h"
#include "os/os.h"
#include "physics/physics_2d.h"
#include "resource/resource_manager.h"

#if defined(ENGINE_ANDROID)
#include "platform/android/android_engine.h"
#elif defined(ENGINE_WIN32)
#include "platform/win32/win32_engine.h"
#endif


void* operator new(size_t size)
{
    FailOn(true, "Avoid 'new' statements!");
    return Engine::data.allocator.alloc(size, alignof(usize)).ptr();
}

void operator delete(void*)
{
    FailOn(true, "Avoid 'delete' statements!");
}

void Engine::initialize()
{
    data.allocator = {};

    data.main_queue = JobQueue::create_with_size(data.allocator.allocator(), DefaultMainQueueSize);
    data.fps = 60;
    data.recreate_requested = false;

    auto allocator = data.allocator.allocator();

    // Going to the assets folder, crash is intended
    FailOn(OS::set_current_directory("assets") == false, "assets directory not found")

    Time::initialize();
    OS::initialize();

    ObjectAllocator::initialize();

    Display::initialize(allocator);

    // Allocating main window
    data.main_window = Window(Display::window_create());

    Audio::initialize(allocator, Audio::DEFAULT_DRIVER);
    Physics2D::initialize(allocator, Physics2D::DEFAULT_DRIVER);
    Graphics::initialize(allocator, Graphics::DEFAULT_DRIVER);

    ResourceManager::initialize(allocator);
    SceneManager::initialize(allocator);

    // default resources
    data.white_texture = GetResource<Texture2D>("white.png");


    Engine::get_main_window().set_size(__configuration__.WindowSize);
    SceneManager::get_display_target().set_size(__configuration__.DisplayTargetSize);
    Engine::set_vsync(__configuration__.VSync);
    
    __preload__();

    // Entry point for app
    SceneManager::change_scene(__configuration__.CreateMainScene());
}

void Engine::shutdown()
{
    SceneManager::shutdown();
    ResourceManager::shutdown();

    Graphics::shutdown();
    Physics2D::shutdown();
    Audio::shutdown();

    Display::shutdown();
    ObjectAllocator::shutdown();

    OS::shutdown();
    Time::shutdown();

    data.main_queue.destroy();
    data.allocator.destroy();
}

void Engine::recreate_window()
{
    Graphics::recreate();
}

void Engine::destroy()
{
    Graphics::destroy();
}

void Engine::step()
{
    SceneManager::step();
    data.main_queue.run();
}

void Engine::handle_input(const InputEvent& event)
{
    SceneManager::scene_handle_input(event);
}

void Engine::request_recreate_window()
{
    data.recreate_requested = true;
    data.main_queue.add_job([]() 
        {
            data.recreate_requested = false;
            Engine::recreate_window();
        }
    );
}

void Engine::set_vsync(bool vsync)
{
    if (data.vsync_cache == vsync)
        return;

    data.vsync_cache = vsync;
    EGL::set_vsync(vsync);
}

