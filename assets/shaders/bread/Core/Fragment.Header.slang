#pragma once


half4 LoadPixel(Sampler2D Texture, float2 UV)
{
    return half4(Texture.Sample(UV));
}

