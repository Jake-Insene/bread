#pragma once
#include "gpu/gpu.h"
#include "graphics/device_object.h"


namespace Graphics
{

struct SamplerInfo
{
    GPU::Filter min_filter;
    GPU::Filter mag_filter;
    GPU::SamplerMipMapMode mipmap_mode;
    GPU::SamplerAddressMode address_mode_u;
    GPU::SamplerAddressMode address_mode_v;
    GPU::SamplerAddressMode address_mode_w;
    f32 mip_lod_bias;
    bool anisotropy_enable;
    f32 max_anisotropy;
    bool compare_enable;
    GPU::CompareOp compare_op;
    f32 min_lod;
    f32 max_lod;
};

struct Sampler : DeviceObject
{
    GPU::SamplerID gpu_sampler;

    void init(mem::Allocator* _allocator, Device* _parent, GPU::DeviceID gpu_device, const SamplerInfo& info);
    void destroy();
};

}
