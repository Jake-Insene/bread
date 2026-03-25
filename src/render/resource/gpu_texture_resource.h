#pragma once
#include "gpu/gpu.h"
#include "render/core/gpu_memory_allocator_types.h"


struct GPUTextureResourceCreateInfo
{
    GPU::TextureType type;
    GPU::TextureFormat format;
    Vector3U extent;
    Slice<u8> pixels;
};

struct GPUTextureResource
{
    GPU::TextureID gpu_texture;
    GPUMemoryAllocationID allocation;
};

using GPUTextureID = ID<u32, struct _GPUTextureTag>;
