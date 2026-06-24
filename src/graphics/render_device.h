#pragma once
#include "gpu/gpu.h"
#include "graphics/gpu_memory_allocator.h"
#include "graphics/gpu_resource_manager.h"



struct RenderDeviceCreateInfo
{
    Mem::Allocator* allocator;
};

struct RenderDevice
{
    struct
    {
        Mem::Allocator* allocator;

        GPU::PhysicalDeviceID physical_device;
        GPU::DeviceID device;

        struct
        {
            GPU::QueueID graphics;
            GPU::QueueID compute;
            GPU::QueueID copy;
            GPU::QueueID present;
        } queues;

        GPUMemoryAllocator gpu_memory_allocator;
        GPUResourceManager gpu_resource_manager;
    } data;

    void initialize(const RenderDeviceCreateInfo& info);
    void shutdown();

    GPU::DeviceID get_device() const { return data.device; }

    GPU::QueueID get_graphics_queue() const { return data.queues.graphics; }
    GPU::QueueID get_compute_queue() const { return data.queues.compute; }
    GPU::QueueID get_copy_queue() const { return data.queues.copy; }
    GPU::QueueID get_present_queue() const { return data.queues.present; }

    GPUMemoryAllocator* get_gpu_memory_allocator() { return &data.gpu_memory_allocator; }
    GPUResourceManager* get_gpu_resource_manager() { return &data.gpu_resource_manager; }
};

