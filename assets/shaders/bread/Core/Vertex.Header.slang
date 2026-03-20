#pragma once
#include "Set.Header.slang"

struct ObjectData
{
    half4 Color;
    float4 VertexPosition;
    float4 InstancePosition;
    float4x4 WorldTransform;
};

float4 InstanceToWorld(in ObjectData Input, float4 InstancePosition)
{
    return mul(Input.WorldTransform, InstancePosition);
}

float4 WorldToClip(float4 WorldPosition)
{
    return mul(FrameInfo.ViewProjection, WorldPosition);
}

