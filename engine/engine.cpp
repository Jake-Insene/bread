#include "engine/engine.h"

#include "display/display.h"
#include "graphics/graphics.h"
#include "graphics/egl/egl.h"
#include "io/resource_manager.h"
#include "objects/scene_manager.h"
#include "objects/object_allocator.h"
#include "os/os.h"
#include "physics/physics_2d.h"

#if defined(ENGINE_ANDROID)
#include "platform/android/android_engine.h"
#elif defined(ENGINE_WIN32)
#include "platform/win32/win32_engine.h"
#endif


void* operator new(size_t size)
{
    FailOn(true, "Avoid 'new' statements!");
}

void operator delete(void*)
{
    FailOn(true, "Avoid 'delete' statements!");
}

extern Object* bread_main();

static Engine::VTable get_engine_vtable()
{
#if defined(ENGINE_ANDROID)
    return AndroidEngine::get_vtable();
#elif defined(ENGINE_WIN32)
    return Win32Engine::get_vtable();
#endif
}

void Engine::initialize()
{
    data.allocator = {};
    data.fps = 60;
    auto allocator = data.allocator.allocator();

    vtable = get_engine_vtable();

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
    
    // Entry point for app
    SceneManager::change_scene(bread_main());
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
    SceneManager::step();
}

void Engine::handle_input(const InputEvent& event)
{
    SceneManager::handle_input(event);
}

void Engine::set_vsync(bool vsync)
{
    EGL::set_vsync(vsync);
}

