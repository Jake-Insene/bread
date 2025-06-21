#include "engine/engine.h"

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
    data.fps = 60;
    data.recreate_requested = false;

    auto allocator = data.allocator.allocator();
    OS::initialize();

    ObjectAllocator::initialize();
    ResourceManager::initialize(allocator);

    Display::initialize(allocator);

    // Allocating main window
    data.main_window = Window(Display::window_create());

    Physics2D::initialize(allocator, Physics2D::DEFAULT_DRIVER);
    Graphics::initialize(allocator, Graphics::DEFAULT_DRIVER);

    // default resources
    data.white_texture = GetResource<Texture2D>("white.png");

    SceneManager::initialize(allocator);

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
    Graphics::shutdown();
    Physics2D::shutdown();
    Display::shutdown();
    ResourceManager::shutdown();
    ObjectAllocator::shutdown();

    OS::shutdown();
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
    if (data.recreate_requested)
    {
        data.recreate_requested = false;
        Engine::recreate_window();
    }

    SceneManager::step();
}

void Engine::handle_input(const InputEvent& event)
{
    SceneManager::_handle_input(event);
}

void Engine::request_recreate_window()
{
    data.recreate_requested = true;
}

void Engine::set_vsync(bool vsync)
{
    EGL::set_vsync(vsync);
}

