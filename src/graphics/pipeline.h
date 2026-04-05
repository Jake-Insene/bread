#pragma once
#include "gpu/gpu.h"
#include "mem/allocator.h"
#include "graphics/device_object.h"
#include "graphics/shader.h"


namespace Graphics
{

struct PipelineInfo
{
    GPU::PipelineBindPoint bind_point;
    Shader shader;
    GPU::VertexInput vertex_input;
	GPU::InputAssembly input_assembly;
    GPU::RasterizerState rasterizer_state;
	GPU::MultisampleState multisample_state;
    GPU::DepthStencilState depth_stencil_state;
	Slice<const GPU::ConstantBlock> constant_blocks;
	Slice<const GPU::DescriptorSetLayoutCreateInfo> set_layout_infos;
    GPU::RenderingInfo rendering_info;
};

struct Pipeline : DeviceObject
{
    GPU::PipelineID pipeline;
    Slice<GPU::DescriptorSetLayoutID> set_layouts;

    void init(const mem::Allocator& _allocator, Device* _parent, GPU::DeviceID gpu_device, const PipelineInfo& info);
    void destroy();

    GPU::DescriptorSetLayoutID get_set_layout(usize set_index);
};

}
