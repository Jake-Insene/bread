#pragma once
#include "gpu/gpu.h"
#include "graphics/device.h"
#include "render_device/core/gpu_memory_allocator.h"
#include "render_device/core/gpu_resource_manager.h"
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

    GPUMemoryAllocator gpu_memory_allocator;
    GPUResourceManager resource_manager;

    Graphics::Device* get_graphics_device() { return &device; }

    GPUMemoryAllocator* get_gpu_memory_allocator() { return &gpu_memory_allocator; }
    GPUResourceManager* get_resource_manager() { return &resource_manager; }

    void initialize(const SystemInitializeInfo& info);
    void shutdown();
};

