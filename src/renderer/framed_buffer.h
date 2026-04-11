#pragma once
#include "graphics/device.h"
#include "render_device/core/gpu_memory_allocator.h"
#include "render_device/core/gpu_memory_allocator_types.h"


struct GPUMemoryAllocator;

struct FramedBufferCreateInfo
{
    mem::Allocator allocator;
    Graphics::Device* graphics_device;
    GPUMemoryAllocator* gpu_memory_allocator;
    usize buffer_size;
    usize frame_count;
    GPU::BufferUsage usage;
};

struct FramedBuffer
{
    mem::Allocator allocator;
    Graphics::Device* graphics_device;
    GPUMemoryAllocator* gpu_memory_allocator;

    GPUMemoryAllocationID buffer_allocation;
    GPUMemoryAllocationID staging_buffer_allocation;

    Array<Graphics::Buffer*> buffers;
    Array<Graphics::Buffer*> staging_buffers;
    usize buffer_size;

    void init(const FramedBufferCreateInfo& info);
    void destroy();
};

