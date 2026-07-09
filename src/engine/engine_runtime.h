#pragma once
#include "audio/audio_service.h"
#include "concurrency/job_queue.h"
#include "display/window.h"
#include "engine/application.h"
#include "graphics/render_device.h"
#include "mem/generic_allocator.h"
#include "resource/texture.h"
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
    
    Mem::GenericAllocator allocator;
    Version engine_version;
    ApplicationInfo application_info;
    Application* application;
    ApplicationState application_state;

    AudioService audio_service;
    Graphics::RenderDevice render_device;
    ResourceManager resource_manager;
    
    GPU::PhysicalDeviceID selected_physical_device;

    JobQueue main_queue;
    
    Window main_window;

    Texture* white_texture;

    struct
    {
        f64 internal_update_time;
        f64 update_time;
        f64 physics_2d_time;
        f64 render_time;
        f64 present_time;
    } debug_time;

    f32 last_time;
    f32 time_accum;

    i32 fps;
    i32 fps_counter;
    i32 fps_accum;
    f32 delta_time;

    bool vsync_cache;
    bool can_tick;

    void initialize();
    void shutdown();
    
    void step();
    void handle_event(const Event& event);

    void request_recreate_window();

    // Utility functions
    AudioService* get_audio_service() { return &audio_service; }
    Graphics::RenderDevice* get_render_device() { return &render_device; }
    ResourceManager* get_resource_manager() { return &resource_manager; }

    GPU::PhysicalDeviceID get_selected_gpu_device() const { return selected_physical_device; }

    i32 get_fps() const { return fps; }

	ApplicationInfo& get_application_info() { return application_info; }

    Window* get_main_window() { return &main_window; }

    void set_vsync(bool vsync);
    bool get_vsync() const { return vsync_cache; }

    template<typename Fn> 
    void add_main_job(Fn fn)
    {
        main_queue.add_job(fn);
    }

    void _select_physical_device();
};
