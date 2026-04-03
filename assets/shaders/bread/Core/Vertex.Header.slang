#pragma once
#include "Structures.Header.slang"

struct ObjectData
{
    half4 Color;
    float2 UV;
    float4 VertexPosition;
    float4 InstancePosition;
    float4x4 WorldTransform;
};

float4 InstanceToWorld(in ObjectData Input, float4 InstancePosition)
{
    return mul(Input.WorldTransform, InstancePosition);
}

float4 WorldToClip(in FrameInfo Frame, float4 WorldPosition)
{
    return mul(Frame.ViewProjection, WorldPosition);
}

