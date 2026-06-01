#pragma once
#include "math/mat4.h"
#include "gpu/gpu.h"



namespace SceneRenderer
{

struct SceneUniform
{
    Mat4 view;
    Mat4 projection;
    Mat4 view_projection;
};

static constexpr GPU::DescriptorBinding GlobalSceneSet[] =
{
    { .type = GPU::DescriptorType::UniformBuffer, .binding = 0, .count = 1, .stages = GPU::ShaderStage::Vertex | GPU::ShaderStage::Fragment, },
};


}