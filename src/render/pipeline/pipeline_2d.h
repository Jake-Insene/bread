#pragma once
#include "render/pipeline/pipeline.h"


struct Pipeline2D : Pipeline
{
    static Pipeline2D make_default(Slice<const GPU::ShaderStageInfo> stages,
        const GPU::VertexInput& vertex_input, Slice<const GPU::ConstantBlock> constant_blocks,
        Slice<const GPU::DescriptorSetLayoutCreateInfo> set_layout_infos, GPU::SurfaceFormat surface_format);
};