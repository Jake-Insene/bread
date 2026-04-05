#pragma once
#include "graphics/pipeline.h"


namespace Graphics
{

struct Pipeline2DInfo
{
    Shader shader;
    GPU::VertexInput vertex_input;
    Slice<const GPU::ConstantBlock> constant_blocks;
    Slice<const GPU::DescriptorSetLayoutCreateInfo> set_layout_infos;
    GPU::RenderingInfo rendering_info;
};

struct Pipeline2D : Pipeline
{
    static PipelineInfo make_default(const Pipeline2DInfo& info);
};

}
