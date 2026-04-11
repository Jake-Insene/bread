#pragma once
#include "collections/free_list.h"
#include "gpu/gpu.h"
#include "graphics/device.h"
#include "render_device/resource/gpu_resource_types.h"
#include "render_device/core/gpu_memory_allocator_types.h"


struct GPUMemoryAllocator;

struct GPUResourceManagerCreateInfo
{
    mem::Allocator allocator;
    Graphics::Device* graphics_device;
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
        GPU::TextureID gpu_texture;
        GPUMemoryAllocationID allocation;
    };

    mem::Allocator allocator;
    Graphics::Device* graphics_device;
    GPUMemoryAllocator* gpu_memory_allocator;

    FreeList<TextureData, GPUTextureID> textures;

    void init(const GPUResourceManagerCreateInfo& info);
    void destroy();

    GPUTextureID create_texture(const TextureAllocateInfo& alloc_info);
    void destroy_texture(GPUTextureID texture_ref);

    GPU::TextureID texture_get_gpu_texture(GPUTextureID texture_ref);

    template<typename Fn>
    void submit_and_wait(GPU::QueueID gpu_queue, Fn&& fn)
    {
        SubmitFn recorder = [](void* arg, GPU::CommandBufferID cmd)
        {
            (*reinterpret_cast<Fn*>(arg))(cmd);
        };
        _submit_and_wait(gpu_queue, reinterpret_cast<void*>(&fn), recorder);
    }

    void _submit_and_wait(GPU::QueueID gpu_queue, void* arg, SubmitFn recorder);
};
