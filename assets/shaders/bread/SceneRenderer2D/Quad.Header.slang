
#include "bread/Core/Header.slang"

static float2 QuadPositions[6] = float2[6](
    float2(0, 1),
    float2(1, 1),
    float2(1, 0),

    float2(1, 0),
    float2(0, 0),
    float2(0, 1)
);

struct VertexInput
{
    float4 Attrib0 : POSITION;
    float4 Attrib1 : UV;
    float4 Attrib2 : COLOR;
};

struct FragmentInput
{
    nointerpolation half4 Color;
    float4 ClipPosition : SV_Position;
};

FragmentInput Vertex(ObjectData Object);

[shader("vertex")]
FragmentInput VertexMain(VertexInput Input, uint VertexIndex : SV_VertexID)
{
    ObjectData Object;

    uint InputUintColor = reinterpret<uint>(Input.Attrib1.z);
    Object.Color = unpackUnorm4x8ToHalf(InputUintColor);

    float2 VertexPosition = QuadPositions[VertexIndex % 6];
    Object.VertexPosition = float4(VertexPosition, 0, 1);

    Object.InstancePosition = float4((VertexPosition * Input.Attrib2.zw) + Input.Attrib2.xy, 0, 1);
    
    Object.WorldTransform = float4x4(
        float4(Input.Attrib0.xy, 0, Input.Attrib1.x),
        float4(Input.Attrib0.zw, 0, Input.Attrib1.y),
        float4(0, 0, 1, 0),
        float4(0, 0, 0, 1)
    );

    FragmentInput Output = Vertex(Object);
    
    return Output;
}

half4 Fragment(FragmentInput Input);

[shader("fragment")]
half4 FragmentMain(FragmentInput Input) : SV_Target
{
    return Fragment(Input);
}
