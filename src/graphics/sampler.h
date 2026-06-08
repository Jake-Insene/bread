#pragma once
#include "gpu/gpu.h"
#include "graphics/device_object.h"
#include "graphics/structures.h"


namespace Graphics
{

struct Sampler : DeviceObject
{
    GPU::SamplerID gpu_sampler;

    void init(Mem::Allocator* _allocator, Device* _parent, GPU::DeviceID gpu_device, const SamplerInfo& info);
    void destroy();
};

}
