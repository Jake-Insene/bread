#pragma once
#include "math/mat4.h"
#include "math/projection.h"
#include "gpu/gpu.h"



namespace SceneRenderer
{

struct SceneUniform
{
    Mat4 view;
    Mat4 projection;
    Mat4 view_projection;

    static constexpr SceneUniform setup(Vector2I viewport_size)
    {
        SceneUniform scene_uniform = {};
        scene_uniform.view = Mat4::identity();
        scene_uniform.projection = Projection::orthographic(
            0, f32(viewport_size.width),
            0, f32(viewport_size.height),
            0, 1
        );
        scene_uniform.view_projection = scene_uniform.view * scene_uniform.projection;

        scene_uniform.view.transpose();
        scene_uniform.projection.transpose();
        scene_uniform.view_projection.transpose();

        return scene_uniform;
    }
};

static constexpr GPU::DescriptorBinding GlobalSceneSet[] =
{
    { .type = GPU::DescriptorType::UniformBuffer, .binding = 0, .count = 1, .stages = GPU::ShaderStage::Vertex | GPU::ShaderStage::Fragment, },
};

}