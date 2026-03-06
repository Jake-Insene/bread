#version 450

#pragma shader_stage(vertex)
#define VERTEX_SHADER
#include "core/header.glsl"


vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);


void main()
{
    gl_Position = vec4(positions[VertexIndex], 0, 1);
}
