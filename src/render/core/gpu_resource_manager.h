#pragma once
#include "collections/free_list.h"
#include "render/resource/gpu_texture_resource.h"


struct RenderDevice;
struct GPUMemoryAllocator;

struct GPUResourceManager
{
    mem::Allocator allocator;

    RenderDevice* render_device;
    GPUMemoryAllocator* memory_allocator;

    FreeList<GPUTextureResource, GPUTextureRef> textures;

    void initialize(const mem::Allocator& _allocator);
    void shutdown();

    GPUTextureRef create_texture(const GPUTextureResourceCreateInfo& ci);
    void destroy_texture(GPUTextureRef texture_ref);
};
