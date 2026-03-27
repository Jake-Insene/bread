#pragma once
#include "collections/free_list.h"
#include "gpu/gpu.h"
#include "render/resource/gpu_render_material.h"
#include "render/resource/gpu_resource_types.h"


struct RenderDevice;
struct GPUMemoryAllocator;


struct GPUMaterialManager
{
    struct MaterialData
    {
        GPU::BufferID ubo;
    };

    mem::Allocator allocator;

    RenderDevice* render_device;
    GPUMemoryAllocator* memory_allocator;

    FreeList<MaterialData, GPURenderMaterialID> materials;

    void initialize(const mem::Allocator& _allocator);
    void shutdown();
};
