#include "engine/engine.h"

#include "audio/audio.h"
#include "display/display.h"
#include "graphics/graphics.h"
#include "graphics/render_manager.h"
#include "graphics/egl/egl.h"
#include "log/log.h"
#include "object/object_allocator.h"
#include "scene/scene_manager.h"
#include "os/os.h"
#include "physics/physics_2d.h"
#include "resource/resource_manager.h"


void* operator new(size_t)
{
    FailOn(true, "Avoid 'new' statements!");
    return (void*)0xFFFFFFFF'FFFFFFFF;
}

void operator delete(void*)
{
    FailOn(true, "Avoid 'delete' statements!");
}

void Engine::initialize()
{
    data.allocator = {};
    auto allocator = data.allocator.allocator();

    // Initilizing the core components
    Log::debug("[Engine]: Initializing...");

    // To use thread and mutexes.
    OS::initialize(allocator);

    data.main_queue = JobQueue::create_with_size(allocator, DefaultMainQueueSize);
    data.fps = 60;

    // Going to the assets folder, crash is intended
    FailOn(OS::set_current_directory("assets") == false, "assets directory not found")

    ObjectAllocator::initialize();

    Display::initialize(allocator);

    // Allocating main window
    data.main_window = Window(Display::window_create());

    Audio::initialize(allocator, Audio::DEFAULT_DRIVER);
    Graphics::initialize(allocator);
    RenderManager::initialize(allocator);

    SceneManager::initialize(allocator);
    Physics2D::initialize(allocator, Physics2D::DEFAULT_DRIVER);

    ResourceManager::initialize(allocator);

    Engine::get_main_window().set_size(__configuration__.viewport_size);
    SceneManager::set_keep_viewport(__configuration__.keep_viewport);
    SceneManager::set_viewport_size(__configuration__.viewport_size);
    Engine::set_vsync(__configuration__.vsync);
    
    __preload__();

    // Entry point for app
    SceneManager::change_scene(__configuration__.create_main_scene());
}

void Engine::shutdown()
{
    SceneManager::shutdown();
    Physics2D::shutdown();

    ResourceManager::shutdown();

    RenderManager::shutdown();
    Graphics::shutdown();
    Audio::shutdown();

    Display::shutdown();
    ObjectAllocator::shutdown();

    data.main_queue.destroy();

    OS::shutdown();

    data.allocator.destroy();
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
    data.main_queue.add_job([]() 
        {
            SceneManager::recreate_window();
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

