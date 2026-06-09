#pragma once
#include "collections/free_list.h"
#include "graphics/device.h"
#include "render_device/resource/gpu_resource_types.h"
#include "render_device/core/gpu_memory_allocator_types.h"


struct GPUMemoryAllocator;

struct GPUResourceManagerCreateInfo
{
    Mem::Allocator* allocator;
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
        Graphics::Texture* texture;
        Graphics::TextureView* texture_view;
        GPUMemoryAllocationID allocation;
    };

    Mem::Allocator* allocator;
    Graphics::Device* graphics_device;
    GPUMemoryAllocator* gpu_memory_allocator;

    FreeList<TextureData, GPUTextureID> textures;

    void init(const GPUResourceManagerCreateInfo& info);
    void destroy();

    GPUTextureID create_texture(const TextureAllocateInfo& alloc_info);
    void destroy_texture(GPUTextureID texture_ref);

    Graphics::Texture* texture_get_texture(GPUTextureID texture_ref);
    Graphics::TextureView* texture_get_texture_view(GPUTextureID texture_ref);

    template<typename Fn>
    void submit_and_wait(Graphics::Queue* queue, Fn&& fn)
    {
        SubmitFn recorder = [](void* arg, GPU::CommandBufferID cmd)
        {
            (*reinterpret_cast<Fn*>(arg))(cmd);
        };
        _submit_and_wait(queue, reinterpret_cast<void*>(&fn), recorder);
    }

    void _submit_and_wait(Graphics::Queue* queue, void* arg, SubmitFn recorder);
};
