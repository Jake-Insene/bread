#include "engine/engine.h"

#include "audio/audio.h"
#include "display/display.h"
#include "graphics/graphics.h"
#include "log/log.h"
#include "scene/scene_manager.h"
#include "os/os.h"
#include "render/render_manager.h"
#include "physics/physics_2d.h"
#include "resource/resource_manager.h"


void* operator new(size_t)
{
    FailOn(true, "avoid 'new' statements!");
    return reinterpret_cast<void*>(0xFFFFFFFF'FFFFFFFF);
}

void operator delete(void*)
{
    FailOn(true, "avoid 'delete' statements!");
}

void Engine::initialize()
{
    data.allocator = {};
    mem::Allocator allocator = data.allocator.allocator();

    // Initilizing the core components
    Log::debug("[Engine]: Initializing...");

    // To use thread and mutexes.
    OS::initialize(allocator);

    data.main_queue = JobQueue::with_size(allocator, DefaultMainQueueSize);
    data.fps = 60;

    // Going to the assets folder, crash is intended for now
    // TODO: Find a better way to handle this.
    FailOn(OS::set_current_directory("assets") == false, "assets directory not found")

    Graphics::initialize(allocator);
    Display::initialize(allocator);

    // Allocating main window
    data.main_window = Window(Display::window_create());

    Audio::initialize(allocator, Audio::DEFAULT_DRIVER);
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
    SceneManager::change_scene(__configuration__.create_main_scene(allocator));
}

void Engine::shutdown()
{
    SceneManager::shutdown();
    Physics2D::shutdown();

    ResourceManager::shutdown();

    RenderManager::shutdown();
    Audio::shutdown();
    
    data.main_window.destroy();
    
    Display::shutdown();
    Graphics::shutdown();

    data.main_queue.destroy();

    OS::shutdown();

    data.allocator.destroy();
}

void Engine::destroy()
{
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
    // TODO: Set VSync
}

