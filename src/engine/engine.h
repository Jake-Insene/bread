#pragma once
#include "concurrency/job_queue.h"
#include "display/window.h"
#include "engine/engine_runtime.h"
#include "engine/configuration.h"
#include "systems/system_manager.h"



struct Scene;
struct InputEvent;

struct Engine
{
    static constexpr usize DefaultMainQueueSize = 16;

    struct InternalData
    {
        EngineRuntime* engine_runtime;
    };

    static inline InternalData local_data = {};

    static SystemManager& get_system_manager()
    {
        return local_data.engine_runtime->get_system_manager();
    }

    static GPU::PhysicalDeviceID get_selected_gpu_device()
    {
        return local_data.engine_runtime->get_selected_gpu_device();
    }
    
    static i32 get_fps()
    {
        return local_data.engine_runtime->get_fps();
    }

	static EngineConfiguration& get_configuration()
    {
        return local_data.engine_runtime->get_configuration();
    }

    static Window get_main_window()
    {
        return local_data.engine_runtime->get_main_window();
    }

    static void set_vsync(bool vsync)
    {
        local_data.engine_runtime->set_vsync(vsync);
    }

    static bool get_vsync()
    {
        return local_data.engine_runtime->get_vsync();
    }

    template<typename Fn> 
    static void add_main_job(Fn fn)
    {
        local_data.engine_runtime->add_main_job(fn);
    }
};
