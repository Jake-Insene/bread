#version 450 core

#pragma shader_stage(fragment)
#define FRAGMENT_SHADER
#include "core/header.glsl"


layout(location = 0) in vec2 InputUV;
layout(location = 0) out vec4 FragmentColor;

layout(set = 0, binding = 1) uniform sampler2D texture0;

void main()
{
    FragmentColor = LoadPixel(texture0, InputUV);
}