#pragma once
#include "io/resource.h"
#include "math/vec2.h"


// Represents an collection of pixels in memory.
struct Image : Resource
{
    enum ImageFormat
    {
        FORMAT_UNKNOWN = 0,
        FORMAT_RGB8,
        FORMAT_RGBA8,
    };
    
    Slice<u8> pixels;
    Vector2I size;
    ImageFormat format;
    
    void destroy();
    
    [[nodiscard]] bool load(StringView path);
    void unload();
};

