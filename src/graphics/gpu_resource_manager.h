#pragma once
#include "collections/free_list.h"
#include "gpu/gpu.h"
#include "graphics/gpu_resource_types.h"
#include "graphics/gpu_memory_allocator_types.h"


struct GPUMemoryAllocator;

struct GPUResourceManagerCreateInfo
{
    Mem::Allocator* allocator;
    GPU::DeviceID device;
    GPU::QueueID graphics_queue;
    GPU::QueueID copy_queue;
    GPUMemoryAllocator* gpu_memory_allocator;   
};

struct GPUResourceManager
{
    using SubmitFn = void(*)(void* arg, GPU::CommandBufferID);

    struct TextureAllocateInfo
    {
        GPU::TextureType type;
        GPU::TextureFormat format;
        Vector3U extent;
        Slice<u8> pixels;
    };

    struct TextureData
    {
        GPU::TextureID texture;
        GPU::TextureViewID texture_view;
        GPUMemoryAllocationID allocation;
    };

    struct InternalData
    {
        Mem::Allocator* allocator;
        GPU::DeviceID device;
        GPU::QueueID graphics_queue;
        GPU::QueueID copy_queue;

        GPUMemoryAllocator* gpu_memory_allocator;  

        FreeList<TextureData, GPUTextureID> textures;
    } data;

    void init(const GPUResourceManagerCreateInfo& info);
    void destroy();

    GPUTextureID create_texture(const TextureAllocateInfo& alloc_info);
    void destroy_texture(GPUTextureID texture_ref);

    GPU::TextureID texture_get_texture(GPUTextureID texture_ref);
    GPU::TextureViewID texture_get_texture_view(GPUTextureID texture_ref);

    template<typename Fn>
    void submit_and_wait(GPU::QueueID queue, GPU::QueueUsage usage, Fn&& fn)
    {
        SubmitFn recorder = [](void* arg, GPU::CommandBufferID cmd)
        {
            (*reinterpret_cast<Fn*>(arg))(cmd);
        };
        _submit_and_wait(queue, usage, reinterpret_cast<void*>(&fn), recorder);
    }

    void _submit_and_wait(GPU::QueueID queue, GPU::QueueUsage usage, void* arg, SubmitFn recorder);
};
