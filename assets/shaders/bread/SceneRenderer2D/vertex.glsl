#version 450

#pragma shader_stage(vertex)
#define VERTEX_SHADER
#include "core/header.glsl"

layout(location = 0) in vec2 InputPosition;
layout(location = 1) in vec3 InputColor;

layout(location = 0) out vec4 OutputColor;

layout(push_constant, std430) uniform PushConstants
{
    mat4 transform;
};

layout(set = 0, binding = 0) uniform UniformBuffer
{
    mat4 projection;
};

void main()
{
    vec4 position = transform * vec4(InputPosition, 0, 1);
    position = projection * position;
    gl_Position = position;
    OutputColor = vec4(InputColor, 1);
}
