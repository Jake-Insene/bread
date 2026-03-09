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

void main()
{
    gl_Position = transform * vec4(InputPosition, 0, 1);
    OutputColor = vec4(InputColor, 1);
}
