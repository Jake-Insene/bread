#include "graphics/sampler.h"



namespace Graphics
{

void Sampler::init(mem::Allocator* _allocator, Device* _parent, GPU::DeviceID gpu_device, const SamplerInfo& info)
{
    DeviceObject::init(_allocator, _parent);
    gpu_sampler = GPU::sampler_create(
        {
            .device = gpu_device,
            .min_filter = info.min_filter,
            .mag_filter = info.mag_filter,
            .mipmap_mode = info.mipmap_mode,
            .address_mode_u = info.address_mode_u,
            .address_mode_v = info.address_mode_v,
            .address_mode_w = info.address_mode_w,
            .mip_lod_bias = info.mip_lod_bias,
            .anisotropy_enable = info.anisotropy_enable,
            .max_anisotropy = info.max_anisotropy,
            .compare_enable = info.compare_enable,
            .compare_op = info.compare_op,
            .min_lod = info.min_lod,
            .max_lod = info.max_lod,
        }
    );
}

void Sampler::destroy()
{
    GPU::sampler_destroy(gpu_sampler);
}

}
