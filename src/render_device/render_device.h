#pragma once
#include "gpu/gpu.h"
#include "graphics/device.h"
#include "render_device/core/gpu_memory_allocator.h"
#include "render_device/core/gpu_resource_manager.h"



struct RenderDeviceCreateInfo
{
    Mem::Allocator* allocator;
};

struct RenderDevice
{
    Mem::Allocator* allocator;

    Graphics::Device device;

    GPUMemoryAllocator gpu_memory_allocator;
    GPUResourceManager gpu_resource_manager;

    Graphics::Device* get_graphics_device() { return &device; }

    GPUMemoryAllocator* get_gpu_memory_allocator() { return &gpu_memory_allocator; }
    GPUResourceManager* get_gpu_resource_manager() { return &gpu_resource_manager; }

    void initialize(const RenderDeviceCreateInfo& info);
    void shutdown();
};

