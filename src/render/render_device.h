#pragma once
#include "gpu/gpu.h"
#include "render/core/gpu_memory_allocator.h"
#include "render/core/gpu_resource_manager.h"
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

    mem::Allocator allocator;

    GPU::DeviceID gpu_device;

    GPU::QueueID graphics_queue;
    GPU::QueueID compute_queue;
    GPU::QueueID copy_queue;
    GPU::QueueID present_queue;

    GPUMemoryAllocator memory_allocator;
    GPUResourceManager resource_manager;

    void initialize(const SystemInitializeInfo& info);
    void shutdown();

    void on_event(const InputEvent&) {}

    GPU::DeviceID get_graphics_device() { return gpu_device; }
    GPU::QueueID get_graphics_queue() { return graphics_queue; }
    GPU::QueueID get_compute_queue() { return compute_queue; }
    GPU::QueueID get_copy_queue() { return copy_queue; }
    GPU::QueueID get_present_queue() { return present_queue; }

    GPUMemoryAllocator& get_memory_allocator() { return memory_allocator; }
    GPUResourceManager& get_resource_manager() { return resource_manager; }

    using SubmitFn = void(*)(void* arg, GPU::CommandBufferID);

    template<typename Fn>
    void submit_and_wait(GPU::QueueID queue, Fn&& fn)
    {
        SubmitFn recorder = [](void* arg, GPU::CommandBufferID cmd)
        {
            (*reinterpret_cast<Fn*>(arg))(cmd);
        };
        _submit_and_wait(queue, reinterpret_cast<void**>(&fn), recorder);
    }

    void _submit_and_wait(GPU::QueueID queue, void* arg, SubmitFn recorder);
};

