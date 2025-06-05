#pragma once
#include "resource/resource.h"
#include "math/vec2.h"


// Represents a collection of pixels in memory.
struct Image : Resource
{
    RESOURCE(RESOURCE_IMAGE, .LoadFromAssets = true, .Extensions = "png");

    enum ImageFormat
    {
        FORMAT_UNKNOWN = 0,
        FORMAT_RGB8,
        FORMAT_RGBA8,
    };
    
    Slice<u8> pixels;
    Vector2I size;
    ImageFormat format;
    
    void init();
    void destroy();
    
    [[nodiscard]] bool load(StringView path);
    void unload();
};

