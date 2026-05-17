#pragma once
#include "graphics/device.h"
#include "render_device/core/gpu_memory_allocator.h"
#include "render_device/core/gpu_memory_allocator_types.h"


struct GPUMemoryAllocator;

struct FramedBufferCreateInfo
{
    mem::Allocator* allocator;
    Graphics::Device* graphics_device;
    GPUMemoryAllocator* gpu_memory_allocator;
    usize buffer_size;
    u32 frame_count;
    GPU::BufferUsage usage;
};

struct FramedBuffer
{
    struct BufferInfo
    {
        usize offset;
    };

    mem::Allocator* allocator;
    Graphics::Device* graphics_device;
    GPUMemoryAllocator* gpu_memory_allocator;

    usize buffer_size;
    Array<BufferInfo> buffers_info;

    void init(const FramedBufferCreateInfo& info);
    void destroy();

    BufferInfo get_buffer_info(usize frame_index) const { return buffers_info.get(frame_index); }
};

struct FramedDeviceBuffer : FramedBuffer
{
    GPUMemoryAllocationID buffer_allocation;
    Graphics::Buffer* buffer;

    GPUMemoryAllocationID staging_allocation;
    Graphics::Buffer* staging_buffer;
    Slice<u8> mapped_staging;

    void init(const FramedBufferCreateInfo& info);
    void destroy();

    Slice<u8> get_mapped_staging(usize frame_index)
    {
        return mapped_staging.add(get_buffer_info(frame_index).offset);
    }

    Graphics::Buffer* get_buffer() const { return buffer; }
    Graphics::Buffer* get_staging_buffer() const { return staging_buffer; }
};

struct FramedMappedBuffer : FramedBuffer
{
    GPUMemoryAllocationID mapped_buffer_allocation;
    Graphics::Buffer* mapped_buffer;
    Slice<u8> mapped;

    void init(const FramedBufferCreateInfo& info);
    void destroy();

     Slice<u8> get_mapped(usize frame_index)
    {
        return mapped.add(get_buffer_info(frame_index).offset);
    }

    Graphics::Buffer* get_buffer() const { return mapped_buffer; }
};

