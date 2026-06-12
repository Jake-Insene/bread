#pragma once
#include "renderer/framed_buffer.h"
#include "renderer/scene_renderer.h"
#include "renderer/renderer.h"


struct GPUMemoryAllocator;

struct Renderer2D : Renderer
{
    GPU::SamplerID sampler;

    void init(const RendererCreateInfo& info);
    void destroy();
};
