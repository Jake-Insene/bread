#pragma once
#include "gpu/gpu.h"
#include "render/core/gpu_memory_allocator.h"


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
    GPUMemoryAllocator::AllocationID allocation;
};

using GPUTextureRef = ID<u32, struct _GPUTextureTag>;
