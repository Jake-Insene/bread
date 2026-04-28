#pragma once
#include "graphics/pipeline.h"


namespace Graphics
{

struct Pipeline2DInfo
{
    Shader shader;
    GPU::VertexInput vertex_input;
    GPU::PrimitiveTopology primitive_topology;
    PipelineLayout* pipeline_layout;
    GPU::RenderingInfo rendering_info;
};

struct Pipeline2D : Pipeline
{
    static PipelineInfo make_default(const Pipeline2DInfo& info);
};

}
