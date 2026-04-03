#pragma once

half4 LoadPixelWithSampler(Texture2D Texture, SamplerState Sampler, float2 UV)
{
    return half4(Texture.Sample(Sampler, UV));
}

half4 LoadPixel(Sampler2D Texture, float2 UV)
{
    return half4(Texture.Sample(UV));
}

