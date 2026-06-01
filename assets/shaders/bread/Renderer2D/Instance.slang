#include "bread/Renderer2D/Instance.Header.slang"

#if defined(INSTANCE_SPRITE)
layout(set = 1, binding = 0) Sampler2D BindTextures[16];
#endif

FragmentInput Vertex(ObjectData Object)
{
    FragmentInput Output;

    Output.Color = Object.Color;
    Output.UV = Object.UV;
    float4 WorldPosition = InstanceToWorld(Object, Object.InstancePosition);
    float4 WorldToClip = WorldToClip(Scene, WorldPosition);
    Output.ClipPosition = WorldToClip;
    
    return Output;
}

half4 Fragment(FragmentInput Input) : SV_Target
{
#if defined(INSTANCE_SPRITE)
    return Input.Color * LoadPixel(BindTextures[Input.TextureIndex], Input.UV);
#elif defined(INSTANCE_QUAD) || defined(INSTANCE_LINE)
    return Input.Color;
#elif defined(INSTANCE_CIRCLE)
    half Dist = distance(half2(Input.LocalPosition), half2(0, 0));
    return half4(Input.Color.xyz, Dist > 1.0H ? 0.0H : 1.0H);
#endif
}
