#define INSTANCE_SPRITE
#include "bread/Renderer2D/Instance.Header.slang"
#include "bread/Renderer2D/Renderer2D.Header.slang"

DECLARE_SCENE_UNIFORM(SceneUniform, Scene);
layout(set = 1, binding = 0) Sampler2D BindTextures[16];

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
    return Input.Color * LoadPixel(BindTextures[Input.TextureIndex], Input.UV);
}
