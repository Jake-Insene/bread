#include "bread/Renderer2D/Instance.Header.slang"

layout(set = 0, binding = 0) ConstantBuffer<FrameInfo> Frame;

FragmentInput Vertex(ObjectData Object)
{
    FragmentInput Output;

    Output.Color = Object.Color;
    Output.UV = Object.UV;
    float4 WorldPosition = InstanceToWorld(Object, Object.InstancePosition);
    float4 WorldToClip = WorldToClip(Frame, WorldPosition);
    Output.ClipPosition = WorldToClip;
    
    return Output;
}

half4 Fragment(FragmentInput Input) : SV_Target
{
    return Input.Color; //* LoadPixel(BindTexture, Input.UV);
}
