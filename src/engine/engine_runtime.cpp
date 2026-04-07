#include "engine/engine_runtime.h"

#include "audio/audio.h"
#include "display/display.h"
#include "gpu/gpu.h"
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

void EngineRuntime::initialize()
{
    allocator = {};
    mem::Allocator allocator_ref = allocator.allocator();

    // Initilizing the core components
    Log::debug("[Engine]: Initializing...");

    // To use thread and mutexes.
    OS::initialize(allocator_ref);

    // Initializing systems manager
    system_manager.initialize(allocator_ref);

    main_queue = JobQueue::with_size(allocator_ref, DefaultMainQueueSize);
    fps = 60;

    // Going to the assets folder, crash is intended for now
    // TODO: Find a better way to handle this.
    FailOn(OS::set_current_directory("assets") == false, "assets directory not found")

    GPU::initialize(allocator_ref);
    Display::initialize(allocator_ref);

    _select_physical_device();

    // Allocating main window
    main_window = Window(Display::window_create());

    Audio::initialize(allocator_ref, Audio::DriverType::Default);

    scene_manager.initialize(allocator_ref);
    Physics2D::initialize(allocator_ref, Physics2D::DEFAULT_DRIVER);

    // Initialize subsystems first
    system_manager.allocate_systems(__get_requested_systems__());

    main_window.set_size(__configuration__.viewport_size);
    scene_manager.set_keep_viewport(__configuration__.keep_viewport);
    scene_manager.set_viewport_size(__configuration__.viewport_size);
    set_vsync(__configuration__.vsync);
    
    __preload__();

    // Entry point for app
    scene_manager.change_scene(__configuration__.create_main_scene(allocator_ref));

    can_tick = true;
}

void EngineRuntime::shutdown()
{
    scene_manager.shutdown();

    system_manager.deallocate_systems();

    Physics2D::shutdown();
    Audio::shutdown();
    
    main_window.destroy();
    
    Display::shutdown();
    GPU::shutdown();

    main_queue.destroy();

    OS::shutdown();

    system_manager.shutdown();

    allocator.destroy();
}

void EngineRuntime::step()
{
    if(!can_tick)
    {
        return;
    }
    
    system_manager.tick();
    scene_manager.step();
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
    else if(event.type == InputEventType::WindowClose)
    {
        can_tick = false;
    }
    
    system_manager.tick_event(event);
    scene_manager.scene_handle_event(event);
}

void EngineRuntime::request_recreate_window()
{
    main_queue.add_job([&]() 
        {
            scene_manager.recreate_window();
        }
    );
}

void EngineRuntime::set_vsync(bool vsync)
{
    if (vsync_cache == vsync)
    {
        return;
    }

    vsync_cache = vsync;
    // TODO: Set VSync
}

void EngineRuntime::_select_physical_device()
{
    Slice<GPU::PhysicalDeviceID> physical_devices = GPU::physical_devices_enumerate();

    bool finded = false;
    GPU::PhysicalDeviceID integrated = GPU::PhysicalDeviceID();
    GPU::PhysicalDeviceID cpu = GPU::PhysicalDeviceID();
    for(GPU::PhysicalDeviceID pd : physical_devices)
    {
        if(finded == true)
            break;

        GPU::PhysicalDeviceInfo pd_info = GPU::physical_device_get_info(pd);
        if(pd_info.device_type == GPU::DeviceType::DiscreteGPU)
        {
            physical_device = pd;
            finded = true;
            break;
        }
        else if(pd_info.device_type == GPU::DeviceType::IntegratedGPU)
        {
            integrated = pd;
        }
        else if(pd_info.device_type == GPU::DeviceType::Cpu)
        {
            cpu = pd;
        }
    }

    if(finded == false && integrated.is_valid())
    {
        physical_device = integrated;
    }
    else if(finded == false && !integrated.is_valid())
    {
        physical_device = cpu;
    }
}

