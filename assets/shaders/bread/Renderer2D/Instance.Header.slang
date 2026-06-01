#pragma once
#include "bread/Core/Header.slang"
#include "bread/Renderer2D/Global.Header.slang"

static float2 QuadPositions[6] = float2[6](
    float2(0, 1),
    float2(1, 1),
    float2(1, 0),

    float2(1, 0),
    float2(0, 0),
    float2(0, 1)
);

static float2 UVPositions[6] = float2[6](
    float2(0, 0),
    float2(1, 0),
    float2(1, 1),

    float2(1, 1),
    float2(0, 1),
    float2(0, 0)
);

#if defined(INSTANCE_SPRITE)
struct VertexInput
{
    float4 Attrib0;
    float4 Attrib1;
    float4 Attrib2;
    float4 Attrib3;
};
#elif defined(INSTANCE_QUAD) || defined(INSTANCE_LINE) || defined(INSTANCE_CIRCLE)
struct VertexInput
{
    float4 Attrib0;
    float4 Attrib1;
    float4 Attrib2;
};
#else
struct VertexInput
{
    // This is garanted
    float4 Attrib0;
    float4 Attrib1;
};
#endif

struct FragmentInput
{
    nointerpolation half4 Color;
    float2 UV;
    float4 ClipPosition : SV_Position;
#if defined(INSTANCE_CIRCLE)
float2 LocalPosition;
#endif
#if defined(INSTANCE_SPRITE)
    nointerpolation uint TextureIndex;
#endif
};

FragmentInput Vertex(ObjectData Object);

[shader("vertex")]
FragmentInput VertexMain(VertexInput Input, uint VertexIndex : SV_VertexID)
{
    ObjectData Object;

    uint InputUintColor = reinterpret<uint>(Input.Attrib1.z);
    Object.Color = unpackUnorm4x8ToHalf(InputUintColor);

    Object.UV = UVPositions[VertexIndex % 6];

#if defined(INSTANCE_SPRITE) || defined(INSTANCE_QUAD) || defined(INSTANCE_CIRCLE)
    float2 VertexPosition = QuadPositions[VertexIndex % 6];
    Object.VertexPosition = float4(VertexPosition, 0, 1);
#elif defined(INSTANCE_LINE)
    Object.VertexPosition = float4(1, 1, 0, 1);
#endif

#if defined(INSTANCE_SPRITE) || defined(INSTANCE_QUAD)
    Object.InstancePosition = float4((VertexPosition * Input.Attrib2.zw) + Input.Attrib2.xy, 0, 1);
#elif defined(INSTANCE_LINE)
    Object.InstancePosition = float4(VertexIndex == 0 ? Input.Attrib2.xy : Input.Attrib2.zw, 0, 1);
#elif defined(INSTANCE_CIRCLE)
    Object.InstancePosition = float4((VertexPosition * Input.Attrib2.zz) + Input.Attrib2.xy, 0, 1);
#endif

    Object.WorldTransform = float4x4(
        float4(Input.Attrib0.xy, 0, Input.Attrib1.x),
        float4(Input.Attrib0.zw, 0, Input.Attrib1.y),
        float4(0, 0, 1, 0),
        float4(0, 0, 0, 1)
    );

    FragmentInput Output = Vertex(Object);
#if defined(INSTANCE_SPRITE)
    Output.TextureIndex = reinterpret<uint>(Input.Attrib1.w);
#endif
#if defined(INSTANCE_CIRCLE)
    Output.LocalPosition = (VertexPosition - float2(0.5, 0.5)) * 2.0;
#endif

    return Output;
}

half4 Fragment(FragmentInput Input);

[shader("fragment")]
half4 FragmentMain(FragmentInput Input) : SV_Target
{
    return Fragment(Input);
}
