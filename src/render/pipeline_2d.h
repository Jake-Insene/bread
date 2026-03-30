#pragma once
#include "render/pipeline.h"


struct Pipeline2DInfo
{
    Shader shader;
    GPU::VertexInput vertex_input;
    Slice<const GPU::ConstantBlock> constant_blocks;
    Slice<const GPU::DescriptorSetLayoutCreateInfo> set_layout_infos;
    GPU::SurfaceFormat surface_format;
};

struct Pipeline2D : Pipeline
{
    static Pipeline2D make_default(const Pipeline2DInfo& info);
};