#pragma once
#include "gpu/gpu.h"
#include "graphics/device.h"
#include "render/core/gpu_memory_allocator.h"
#include "render/core/gpu_resource_manager.h"
#include "systems/system.h"



struct RenderDevice final : System<RenderDevice>
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

    Graphics::Device device;

    GPUMemoryAllocator memory_allocator;
    GPUResourceManager resource_manager;

    Graphics::Device& get_graphics_device() { return device; }

    GPUMemoryAllocator& get_memory_allocator() { return memory_allocator; }
    GPUResourceManager& get_resource_manager() { return resource_manager; }

    void initialize(const SystemInitializeInfo& info);
    void shutdown();

    void on_event(const InputEvent&) {}

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

