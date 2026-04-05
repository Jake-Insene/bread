#pragma once
#include "collections/free_list.h"
#include "render_device/resource/gpu_resource_types.h"
#include "render_device/resource/gpu_texture_resource.h"


struct RenderDevice;
struct GPUMemoryAllocator;

struct GPUResourceManager
{
    mem::Allocator allocator;

    RenderDevice* render_device;
    GPUMemoryAllocator* memory_allocator;

    FreeList<GPUTextureResource, GPUTextureID> textures;

    void initialize(const mem::Allocator& _allocator);
    void shutdown();

    GPUTextureID create_texture(const GPUTextureResourceCreateInfo& ci);
    void destroy_texture(GPUTextureID texture_ref);
};
