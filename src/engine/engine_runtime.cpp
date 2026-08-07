#include "engine/engine_runtime.h"

#include "audio/audio.h"
#include "debug/profiler.h"
#include "display/display.h"
#include "engine/engine.h"
#include "gpu/gpu.h"
#include "input/input.h"
#include "debug/log.h"
#include "os/os.h"
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

EngineRuntime::EngineRuntime(Mem::Allocator& allocator)
: allocator(allocator),
audio_service(allocator), render_device(allocator),
gpu_memory_allocator(
    {
        .allocator = allocator,
        .device = render_device.get_device(),
        .graphics_queue = render_device.get_graphics_queue(),
        .copy_queue = render_device.get_copy_queue(),
    }
),
gpu_resource_manager(
    {
        .allocator = allocator,
        .device = render_device.get_device(),
        .graphics_queue = render_device.get_graphics_queue(),
        .copy_queue = render_device.get_copy_queue(),
        .gpu_memory_allocator = &gpu_memory_allocator,
    }
),
resource_manager(allocator),
main_queue(allocator, DefaultMainQueueSize)
{   
    engine_version = EngineVersion;
    application_info = __get_application_info__();
    application = nullptr;
    application_state = ApplicationState::Unknown;

    // Initilizing the core components
    Log::debug("[Engine]: Initializing...");

    // Going to the assets folder, crash is intended for now
    // TODO: Find a better way to handle this.
    FailOn(OS::set_current_directory("assets") == false, "assets directory not found")

    _select_physical_device();

    // Allocating main window
    main_window = Window(Display::window_create());
    main_window.set_size(get_application_info().initial_window_size);
    
    fps = 0;
    fps_counter = 0;
    fps_accum = 0;
    
    last_time = f32(OS::get_time());
    time_accum = 0;
    delta_time = 0;

    can_tick = true;

    // Entry point for app
    application = reinterpret_cast<Application*>(
        allocator.alloc(
            get_application_info().size_in_bytes,
            get_application_info().alignment
        ).ptr()
    );
    get_application_info().constructor(Opaque::from(*application),
        {
            .allocator = allocator,
            .render_device = get_render_device(),
            .window = get_main_window()->window_id,
        }
    );

    application->initialize(
        {
            .allocator = allocator,
        }
    );
    application_state = ApplicationState::Initialized;

    application->load_resources();
}

EngineRuntime::~EngineRuntime()
{
    application->unload_resources();
    
    application->shutdown();
    DestructObject(*application);
    application_state = ApplicationState::Destroyed;

    allocator.free(Slice(reinterpret_cast<u8*>(application), 1));

    main_window.destroy();
}

void EngineRuntime::pre_step()
{
    Input::update();
}

void EngineRuntime::step()
{
    if(!can_tick)
    {
        return;
    }

    audio_service.update();
    
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
            "\tRender Time: {}\n"
            "\tPresent Time: {}",
            fps_counter, delta_time, debug_time.internal_update_time,
            debug_time.update_time, 
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
            debug_time.render_time = duration;
        );

        application->render();
        fps_accum++;
    }

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
}

GPU::PhysicalDeviceID EngineRuntime::get_selected_gpu_device()
{
    if(selected_physical_device == GPU::PhysicalDeviceID::invalid())
    {
        _select_physical_device();
    }

    return selected_physical_device;
}

void EngineRuntime::_select_physical_device()
{
    Slice physical_devices = GPU::physical_devices_enumerate();

    bool finded = false;
    GPU::PhysicalDeviceID integrated = GPU::PhysicalDeviceID();
    GPU::PhysicalDeviceID cpu = GPU::PhysicalDeviceID();
    for(GPU::PhysicalDeviceID physical_device : physical_devices)
    {
        if(finded)
        {
            break;
        }

        GPU::PhysicalDeviceInfo pd_info = GPU::physical_device_get_info(physical_device);
        if(pd_info.device_type == GPU::DeviceType::DiscreteGPU)
        {
            selected_physical_device = physical_device;
            finded = true;
            break;
        }
        
        if(pd_info.device_type == GPU::DeviceType::IntegratedGPU)
        {
            integrated = physical_device;
        }
        else if(pd_info.device_type == GPU::DeviceType::Cpu)
        {
            cpu = physical_device;
        }
    }

    if(!finded && integrated.is_valid())
    {
        selected_physical_device = integrated;
    }
    else if(!finded && !integrated.is_valid())
    {
        selected_physical_device = cpu;
    }
}


namespace Main
{

void runtime_begin(Mem::Allocator& allocator)
{
    OS::initialize(allocator);
    Display::initialize(allocator);
    Audio::initialize(allocator, Audio::DriverType::Default);
    GPU::initialize(allocator);
}

void runtime_end()
{
    GPU::shutdown();
    Audio::shutdown();
    Display::shutdown();
    OS::shutdown();
}

}
