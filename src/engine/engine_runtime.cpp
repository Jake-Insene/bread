#include "engine/engine_runtime.h"

#include "audio/audio.h"
#include "display/display.h"
#include "graphics/graphics.h"
#include "log/log.h"
#include "scene/scene_manager.h"
#include "os/os.h"
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

void EngineRuntime::init()
{
    allocator = {};
    mem::Allocator allocator_ref = allocator.allocator();

    // Initilizing the core components
    Log::debug("[Engine]: Initializing...");

    // To use thread and mutexes.
    OS::initialize(allocator_ref);

    // Initializing systems manager
    system_manager.init(allocator_ref);

    main_queue = JobQueue::with_size(allocator_ref, DefaultMainQueueSize);
    fps = 60;

    // Going to the assets folder, crash is intended for now
    // TODO: Find a better way to handle this.
    FailOn(OS::set_current_directory("assets") == false, "assets directory not found")

    Graphics::initialize(allocator_ref);
    Display::initialize(allocator_ref);

    _select_physical_device();

    // Allocating main window
    main_window = Window(Display::window_create());

    Audio::initialize(allocator_ref, Audio::DEFAULT_DRIVER);

    SceneManager::initialize(allocator_ref);
    Physics2D::initialize(allocator_ref, Physics2D::DEFAULT_DRIVER);

    ResourceManager::initialize(allocator_ref);

    main_window.set_size(__configuration__.viewport_size);
    SceneManager::set_keep_viewport(__configuration__.keep_viewport);
    SceneManager::set_viewport_size(__configuration__.viewport_size);
    set_vsync(__configuration__.vsync);
    
    __preload__();

    // Initialize subsystems first
    system_manager.allocate_systems(__configuration__.requested_systems);

    // Entry point for app
    SceneManager::change_scene(__configuration__.create_main_scene(allocator_ref));
}

void EngineRuntime::destroy()
{
    SceneManager::shutdown();

    system_manager.deallocate_systems();

    Physics2D::shutdown();

    ResourceManager::shutdown();

    Audio::shutdown();
    
    main_window.destroy();
    
    Display::shutdown();
    Graphics::shutdown();

    main_queue.destroy();

    OS::shutdown();

    system_manager.destroy();

    allocator.destroy();
}

void EngineRuntime::step()
{
    SceneManager::step();
    main_queue.run();
}

void EngineRuntime::handle_event(const InputEvent& event)
{
    if(event.type == InputEventType::WindowResize)
    {
        const InputEventWindowResize& resize_event = event.get<InputEventWindowResize>();
        if(resize_event.window == get_main_window().window_id)
        {
            request_recreate_window();
        }
    }
    SceneManager::scene_handle_event(event);
}

void EngineRuntime::request_recreate_window()
{
    main_queue.add_job([]() 
        {
            SceneManager::recreate_window();
        }
    );
}

void EngineRuntime::set_vsync(bool vsync)
{
    if (vsync_cache == vsync)
        return;

    vsync_cache = vsync;
    // TODO: Set VSync
}

void EngineRuntime::_select_physical_device()
{
    Slice<Graphics::PhysicalDeviceID> physical_devices = Graphics::physical_devices_enumerate();

    bool finded = false;
    Graphics::PhysicalDeviceID integrated = Graphics::PhysicalDeviceID();
    for(Graphics::PhysicalDeviceID pd : physical_devices)
    {
        if(finded == true)
            break;

        Graphics::PhysicalDeviceInfo pd_info = Graphics::physical_device_get_info(pd);
        if(pd_info.device_type == Graphics::DeviceType::DiscreteGPU)
        {
            physical_device = pd;
            finded = true;
            break;
        }
        else if(pd_info.device_type == Graphics::DeviceType::IntegratedGPU)
        {
            integrated = pd;
        }
    }

    if(finded == false)
    {
        physical_device = integrated;
    }
}

