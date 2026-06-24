#pragma once
#include "graphics/gpu_memory_allocator.h"
#include "graphics/gpu_memory_allocator_types.h"


struct GPUMemoryAllocator;

struct FramedBufferCreateInfo
{
    Mem::Allocator* allocator;
    GPU::DeviceID device;
    GPUMemoryAllocator* gpu_memory_allocator;
    u32 frame_count;
    usize buffer_size;
    GPU::BufferUsage usage;
};

struct FramedBuffer
{
    struct BufferInfo
    {
        usize offset;
    };

    struct InternalData
    {
        Mem::Allocator* allocator;
        GPU::DeviceID device;
        GPUMemoryAllocator* gpu_memory_allocator;

        usize buffer_size;
        Array<BufferInfo> buffers_info;
    } data;

    void init(const FramedBufferCreateInfo& info);
    void destroy();

    [[nodiscard]] GPUMemoryAllocator* get_gpu_memory_allocator() const { return data.gpu_memory_allocator; }
    [[nodiscard]] usize get_buffer_size() const { return data.buffer_size; }
    BufferInfo get_buffer_info(usize frame_index) const { return data.buffers_info.get(frame_index); }
};

struct FramedDeviceBuffer : FramedBuffer
{
    using Base = FramedBuffer;

    struct InternalData
    {
        GPUMemoryAllocationID buffer_allocation;
        GPU::BufferID buffer;

        GPUMemoryAllocationID staging_allocation;
        GPU::BufferID staging_buffer;
        Slice<u8> mapped_staging;
    } data;

    void init(const FramedBufferCreateInfo& info);
    void destroy();

    Slice<u8> get_mapped_staging(usize frame_index)
    {
        return data.mapped_staging.add(get_buffer_info(frame_index).offset);
    }

    GPU::BufferID get_buffer() const { return data.buffer; }
    GPU::BufferID get_staging_buffer() const { return data.staging_buffer; }
};

struct FramedMappedBuffer : FramedBuffer
{
    using Base = FramedBuffer;

    struct InternalData
    {
        GPUMemoryAllocationID mapped_buffer_allocation;
        GPU::BufferID mapped_buffer;
        Slice<u8> mapped;
    } data;

    void init(const FramedBufferCreateInfo& info);
    void destroy();

    Slice<u8> get_mapped(usize frame_index)
    {
        return data.mapped.add(get_buffer_info(frame_index).offset);
    }

    GPU::BufferID get_buffer() const { return data.mapped_buffer; }
};

