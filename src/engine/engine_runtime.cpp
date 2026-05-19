#include "engine/engine_runtime.h"

#include "audio/audio.h"
#include "debug/profiler.h"
#include "display/display.h"
#include "engine/engine.h"
#include "gpu/gpu.h"
#include "log/log.h"
#include "os/os.h"
#include "physics/physics_2d.h"
#include "resource/resource_manager.h"


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

void EngineRuntime::initialize()
{

    ConstructObject(allocator);
    engine_version = EngineVersion;
    application_info = __get_application_info__();
    application = nullptr;
    application_state = ApplicationState::Unknown;

    // Initilizing the core components
    Log::debug("[Engine]: Initializing...");

    // To use thread and mutexes.
    OS::initialize(&allocator);

    // Initializing systems manager
    system_manager.initialize(&allocator);

    main_queue = JobQueue::with_size(&allocator, DefaultMainQueueSize);

    // Going to the assets folder, crash is intended for now
    // TODO: Find a better way to handle this.
    FailOn(OS::set_current_directory("assets") == false, "assets directory not found")

    GPU::initialize(&allocator);
    Display::initialize(&allocator);

    _select_physical_device();

    // Allocating main window
    main_window = Window(Display::window_create());

    Audio::initialize(&allocator, Audio::DriverType::Default);
    Physics2D::initialize(&allocator, Physics2D::DEFAULT_DRIVER);

    // Initialize subsystems first
    audio_service.initialize(
        {
            .allocator = &allocator,
        }
    );

    render_device.initialize(
        {
            .allocator = &allocator,
        }
    );

    resource_manager.initialize(
        {
            .allocator = &allocator,
        }
    );

    system_manager.allocate_systems(__get_requested_systems__());
    
    main_window.set_size(get_application_info().viewport_size);
    
    set_vsync(get_application_info().vsync);
    
    __preload__();

    fps = 0;
    fps_counter = 0;
    fps_accum = 0;
    
    last_time = f32(OS::get_time());
    time_accum = 0;
    delta_time = 0;

    vsync_cache = __get_application_info__().vsync;
    can_tick = true;

    // Entry point for app
    application = reinterpret_cast<Application*>(
        allocator.alloc(
            get_application_info().size_in_bytes,
            get_application_info().alignment
        ).ptr()
    );
    get_application_info().constructor(Opaque::from(*application));

    application->initialize(&allocator);
    application_state = ApplicationState::Initialized;
}

void EngineRuntime::shutdown()
{
    application->shutdown();
    application_state = ApplicationState::Destroyed;

    allocator.free(Slice<u8>(reinterpret_cast<u8*>(application), 1));

    resource_manager.shutdown();
    render_device.shutdown();
    audio_service.shutdown();

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
    
    f32 current = f32(OS::get_time());
    delta_time = current - last_time;
    last_time = current;

    time_accum += delta_time;
    if (time_accum >= 1.0)
    {
        fps_counter = fps_accum;
        fps = fps_counter;
        Log::info(
            "Frame Info: FPS: {}\n"
            "\tAvg Frame Time: {}\n"
            "\tInternal Update Time: {}\n"
            "\tUpdate Time: {}\n"
            "\tPhysics 2D Time: {}\n"
            "\tRender Time: {}\n"
            "\tPresent Time: {}",
            fps_counter, delta_time, debug_time.internal_update_time,
            debug_time.update_time, debug_time.physics_2d_time, 
            debug_time.render_time,
            debug_time.present_time
        );

        fps_accum = 0;
        time_accum = 0;
    }

    {
        PROFILE_SCOPE(
            debug_time.update_time = duration;
        );

        application->update(delta_time);
    }

    {
        PROFILE_SCOPE(
            debug_time.physics_2d_time = duration;
        );
        Physics2D::step(delta_time);
    }

    {
        PROFILE_SCOPE(
            debug_time.render_time = duration;
        );

        application->render();
        fps_accum++;
    }

    system_manager.tick();
    main_queue.run();
}

void EngineRuntime::handle_event(const Event& event)
{
    if(event.type == EventType::WindowResize)
    {
        const InputEventWindowResize& resize_event = event.get<InputEventWindowResize>();
        if(resize_event.window == get_main_window()->window_id)
        {
            request_recreate_window();
        }
    }
    else if(event.type == EventType::WindowClose)
    {
        can_tick = false;
    }
    
    if(application_state == ApplicationState::Initialized)
    {
        application->event(event);
    }
}

void EngineRuntime::request_recreate_window()
{
    // TODO:
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
        if(finded)
        {
            break;
        }

        GPU::PhysicalDeviceInfo pd_info = GPU::physical_device_get_info(pd);
        if(pd_info.device_type == GPU::DeviceType::DiscreteGPU)
        {
            physical_device = pd;
            finded = true;
            break;
        }
        
        if(pd_info.device_type == GPU::DeviceType::IntegratedGPU)
        {
            integrated = pd;
        }
        else if(pd_info.device_type == GPU::DeviceType::Cpu)
        {
            cpu = pd;
        }
    }

    if(!finded && integrated.is_valid())
    {
        physical_device = integrated;
    }
    else if(!finded && !integrated.is_valid())
    {
        physical_device = cpu;
    }
}

