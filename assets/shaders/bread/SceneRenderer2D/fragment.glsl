#version 450 core

#pragma shader_stage(fragment)
#define FRAGMENT_SHADER
#include "core/header.glsl"


layout(location = 0) out vec4 FragmentColor;

void main()
{
    FragmentColor = vec4(1, 0, 0, 1);
}