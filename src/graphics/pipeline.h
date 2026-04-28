#pragma once
#include "gpu/gpu.h"
#include "mem/allocator.h"
#include "graphics/device_object.h"
#include "graphics/shader.h"


namespace Graphics
{

struct PipelineLayout;

struct PipelineInfo
{
    GPU::PipelineBindPoint bind_point;
    Shader shader;
    GPU::VertexInput vertex_input;
	GPU::InputAssembly input_assembly;
    GPU::RasterizerState rasterizer_state;
	GPU::MultisampleState multisample_state;
    GPU::DepthStencilState depth_stencil_state;
    PipelineLayout* pipeline_layout;
    GPU::RenderingInfo rendering_info;
};

struct Pipeline : DeviceObject
{
    GPU::PipelineID gpu_pipeline;

    void init(const mem::Allocator& _allocator, Device* _parent, GPU::DeviceID gpu_device, const PipelineInfo& info);
    void destroy();
};

}
