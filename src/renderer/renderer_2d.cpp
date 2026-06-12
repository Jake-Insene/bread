#include "renderer/renderer_2d.h"


void Renderer2D::init(const RendererCreateInfo& info)
{
    Renderer::init(info);

    sampler = GPU::sampler_create(render_device->get_device(),
        GPU::SamplerCreateInfo::create(
            GPU::Filter::Nearest, GPU::Filter::Nearest, GPU::SamplerMipMapMode::Nearest,
            GPU::SamplerAddressMode::Repeat, GPU::SamplerAddressMode::Repeat, GPU::SamplerAddressMode::Repeat,
            0.F, false, 1.F, false, GPU::CompareOp::Always, 0.F, 0.F));
}

void Renderer2D::destroy()
{
    GPU::sampler_destroy(sampler);

    Renderer::destroy();
}

