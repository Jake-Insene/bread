#pragma once
#include "gpu/gpu.h"
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

    GPU::DeviceID gpu_device;

    GPU::QueueID graphics_queue;
    GPU::QueueID compute_queue;
    GPU::QueueID copy_queue;
    GPU::QueueID present_queue;

    void initialize(const SystemInitializeInfo& info);
    void shutdown();

    GPU::DeviceID get_graphics_device() { return gpu_device; }
    GPU::QueueID get_graphics_queue() { return graphics_queue; }
    GPU::QueueID get_compute_queue() { return compute_queue; }
    GPU::QueueID get_copy_queue() { return copy_queue; }
    GPU::QueueID get_present_queue() { return present_queue; }
};

