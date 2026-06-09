#pragma once
#include "gpu/gpu.h"
#include "graphics/device_object.h"
#include "graphics/structures.h"


namespace Graphics
{

struct Pipeline : DeviceObject
{
    GPU::PipelineID gpu_pipeline;

    void init(Mem::Allocator* _allocator, Device* _parent, GPU::DeviceID gpu_device, const PipelineInfo& info);
    void destroy();
};

}
