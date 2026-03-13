#version 450

#pragma shader_stage(vertex)
#define VERTEX_SHADER
#include "core/header.glsl"

layout(location = 0) in vec2 InputPosition;
layout(location = 1) in vec2 InputUV;

layout(location = 0) out vec2 OutputUV;

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
    gl_Position = position;
    OutputUV = InputUV;
}
