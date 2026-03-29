#pragma once
#include "gpu/gpu.h"
#include "mem/allocator.h"


struct PipelineInfo
{
    GPU::DeviceID device;
    GPU::PipelineBindPoint bind_point;
	Slice<const GPU::ShaderStageInfo> shader_stages;
    GPU::VertexInput vertex_input;
	GPU::InputAssembly input_assembly;
    GPU::RasterizerState rasterizer_state;
	GPU::MultisampleState multisample_state;
    GPU::DepthStencilState depth_stencil_state;
	Slice<const GPU::ConstantBlock> constant_blocks;
	Slice<const GPU::DescriptorSetLayoutCreateInfo> set_layout_infos;
    GPU::SurfaceFormat surface_format;
};

struct Pipeline
{
    mem::Allocator allocator;

    GPU::PipelineID pipeline;
    Slice<GPU::DescriptorSetLayoutID> set_layouts;

    static Pipeline create(const PipelineInfo& info);

    void init(const mem::Allocator _allocator, const PipelineInfo& info);
    void destroy();
};
