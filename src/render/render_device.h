#pragma once
#include "graphics/graphics.h"
#include "systems/system.h"



struct RenderDevice : System<RenderDevice>
{
    static constexpr SystemDependency Dependencies[] =
    {
        SystemDependency::of("IdentitySystem")
    };

    static constexpr StringView _name = "RenderDevice";
    static constexpr SystemInfo get_system_info()
    {
        return System::get_system_info_with_name(_name);
    }

    Graphics::DeviceID gpu_device;

    Graphics::QueueID graphics_queue;
    Graphics::QueueID compute_queue;
    Graphics::QueueID copy_queue;
    Graphics::QueueID present_queue;

    void initialize(const SystemInitializeInfo& info);
    void shutdown();

    Graphics::DeviceID get_graphics_device() { return gpu_device; }
    Graphics::QueueID get_graphics_queue() { return graphics_queue; }
    Graphics::QueueID get_compute_queue() { return compute_queue; }
    Graphics::QueueID get_copy_queue() { return copy_queue; }
    Graphics::QueueID get_present_queue() { return present_queue; }
};

