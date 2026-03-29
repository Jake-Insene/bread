#include "render/sampler.h"

#include "engine/engine.h"
#include "render/render_device.h"


Sampler Sampler::create(const SamplerInfo& sampler_info)
{
    Sampler sampler = {};

    RenderDevice* render_device = Engine::get_system_manager()->get_system<RenderDevice>();

    sampler.init(render_device->allocator,
        {
            .device = render_device->get_graphics_device(),
            .min_filter = sampler_info.min_filter,
            .mag_filter = sampler_info.mag_filter,
            .mipmap_mode = sampler_info.mipmap_mode,
            .address_mode_u = sampler_info.address_mode_u,
            .address_mode_v = sampler_info.address_mode_v,
            .address_mode_w = sampler_info.address_mode_w,
            .mip_lod_bias = sampler_info.mip_lod_bias,
            .anisotropy_enable = sampler_info.anisotropy_enable,
            .max_anisotropy = sampler_info.max_anisotropy,
            .compare_enable = sampler_info.compare_enable,
            .compare_op = sampler_info.compare_op,
            .min_lod = sampler_info.min_lod,
            .max_lod = sampler_info.max_lod,
        }
    );
    return sampler;
}


void Sampler::init(const mem::Allocator&, const GPU::SamplerCreateInfo& info)
{
    sampler = GPU::sampler_create(info);
}

void Sampler::destroy()
{
    GPU::sampler_destroy(sampler);
}

