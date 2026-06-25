#pragma once
#include "engine/application.h"
#include "display/window.h"
#include "engine/engine_runtime.h"


struct AudioService;
namespace Graphics
{
struct RenderDevice;
}
struct ResourceManager;
struct SystemManager;

inline constexpr Version EngineVersion = Version::make_variant(0, 1, 0, 0);

struct Engine
{
    static constexpr usize DefaultMainQueueSize = 16;

    struct InternalData
    {
        EngineRuntime* engine_runtime;
    };

    static inline InternalData local_data = {};

    static AudioService* get_audio_service()
    {
        return local_data.engine_runtime->get_audio_service();
    }

    static Graphics::RenderDevice* get_render_device()
    {
        return local_data.engine_runtime->get_render_device();
    }

    static ResourceManager* get_resource_manager()
    {
        return local_data.engine_runtime->get_resource_manager();
    }

    static GPU::PhysicalDeviceID get_selected_gpu_device()
    {
        return local_data.engine_runtime->get_selected_gpu_device();
    }
    
    static i32 get_fps()
    {
        return local_data.engine_runtime->get_fps();
    }

	static ApplicationInfo& get_configuration()
    {
        return local_data.engine_runtime->get_application_info();
    }

    static Window* get_main_window()
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
