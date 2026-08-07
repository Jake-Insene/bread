#pragma once
#include "audio/audio_service.h"
#include "concurrency/job_queue.h"
#include "display/window.h"
#include "engine/application.h"
#include "graphics/render_device.h"
#include "graphics/gpu_memory_allocator.h"
#include "graphics/gpu_resource_manager.h"
#include "resource/resource_manager.h"


struct Event;

extern ApplicationInfo __get_application_info__();

struct EngineRuntime
{
    static constexpr usize DefaultMainQueueSize = 16;

    enum class ApplicationState
    {
        Unknown = 0,
        Initialized,
        Destroyed,
    };
    
    Mem::Allocator& allocator;
    Version engine_version;
    ApplicationInfo application_info;
    Application* application;
    ApplicationState application_state;

    AudioService audio_service;
    Graphics::RenderDevice render_device;
    Graphics::GPUMemoryAllocator gpu_memory_allocator;
    Graphics::GPUResourceManager gpu_resource_manager;
    ResourceManager resource_manager;
    
    GPU::PhysicalDeviceID selected_physical_device;

    JobQueue main_queue;
    
    Window main_window;

    struct
    {
        f64 internal_update_time;
        f64 update_time;
        f64 render_time;
        f64 present_time;
    } debug_time;

    f32 last_time;
    f32 time_accum;

    i32 fps;
    i32 fps_counter;
    i32 fps_accum;
    f32 delta_time;

    bool can_tick;

    EngineRuntime(Mem::Allocator& allocator);
    ~EngineRuntime();
    
    void pre_step();
    void step();
    void handle_event(const Event& event);

    void request_recreate_window();

    // Utility functions
    AudioService* get_audio_service() { return &audio_service; }
    Graphics::RenderDevice* get_render_device() { return &render_device; }
    ResourceManager* get_resource_manager() { return &resource_manager; }
    Graphics::GPUMemoryAllocator* get_gpu_memory_allocator() { return &gpu_memory_allocator; }
    Graphics::GPUResourceManager* get_gpu_resource_manager() { return &gpu_resource_manager; }

    GPU::PhysicalDeviceID get_selected_gpu_device();

	ApplicationInfo& get_application_info() { return application_info; }

    Window* get_main_window() { return &main_window; }

    template<typename Fn> 
    void add_main_job(Fn fn)
    {
        main_queue.add_job(fn);
    }

    void _select_physical_device();
};

namespace Main
{

void runtime_begin(Mem::Allocator& allocator);
void runtime_end();

}
