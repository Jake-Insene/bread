#include "engine/engine.h"

#include "graphics/graphics.h"
#include "graphics/egl/egl.h"
#include "io/resource_manager.h"
#include "objects/scene_manager.h"
#include "objects/object_allocator.h"
#include "physics/physics_2d.h"


void* operator new(size_t)
{
    FailOn(true, "Avoid 'new' statements!");
}

void operator delete(void*)
{
    FailOn(true, "Avoid 'delete' statements!");
}

extern Object* game_create_main_scene();

void Engine::initialize()
{
    data.allocator = {};
    auto allocator = data.allocator.allocator();
    
    ObjectAllocator::initialize();
    ResourceManager::initialize(allocator);
    Physics2D::initialize(allocator, Physics2D::DEFAULT_DRIVER);
    Graphics::initialize(allocator, Graphics::DEFAULT_DRIVER);
    
    // default resources
    data.white_texture = GetResource<Texture2D>("white.png");
    
    SceneManager::initialize(allocator);
    
    // Entry point for app
    SceneManager::change_scene(game_create_main_scene());
}

void Engine::shutdown()
{
    SceneManager::shutdown();
    Graphics::shutdown();
    Physics2D::shutdown();
    ResourceManager::shutdown();
    ObjectAllocator::shutdown();

    data.allocator.destroy();
}

void Engine::recreate_window()
{
    Graphics::recreate();
    SceneManager::data.window_size = EGL::get_surface_size();
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
