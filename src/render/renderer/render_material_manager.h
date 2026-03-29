#pragma once
#include "collections/free_list.h"
#include "gpu/gpu.h"
#include "render/render_types.h"
#include "render/resource/gpu_resource_types.h"


struct RenderDevice;
struct GPUMemoryAllocator;


struct RenderMaterialManager
{
    struct MaterialData
    {
        GPU::BufferID ubo;
    };

    mem::Allocator allocator;

    RenderDevice* render_device;
    GPUMemoryAllocator* memory_allocator;

    FreeList<MaterialData, RenderMaterialID> materials;

    void initialize(const mem::Allocator& _allocator);
    void shutdown();
};
