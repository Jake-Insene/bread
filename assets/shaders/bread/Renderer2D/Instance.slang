#include "bread/Renderer2D/Instance.Header.slang"

layout(set = 0, binding = 0) ConstantBuffer<FrameInfo> Frame;
#if defined(INSTANCE_SPRITE)
layout(set = 0, binding = 1) Sampler2D BindTexture;
#endif

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
#if defined(INSTANCE_SPRITE)
    return Input.Color * LoadPixel(BindTexture, Input.UV);
#elif defined(INSTANCE_QUAD) || defined(INSTANCE_LINE)
    return Input.Color;
#elif defined(INSTANCE_CIRCLE)
    return Input.Color;
#endif
}
