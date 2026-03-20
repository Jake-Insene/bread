
struct FrameUB
{
    float4x4 View;
    float4x4 Projection;
    float4x4 ViewProjection;
};

layout(set = 0, binding = 0) ConstantBuffer<FrameUB> FrameInfo;
