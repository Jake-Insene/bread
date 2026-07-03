#pragma once
#include "collections/error.h"
#include "resource/resource.h"
#include "math/vec2.h"


/*
* Represents a collection of pixels in memory.
*/
struct Image : Resource
{
    RESOURCE(RESOURCE_IMAGE, ResourceFlags::LoadFromAssets, ResourceExtensions("png"))

    enum class ImageFormat
    {
        Unknown = 0,
        RGB8,
        RGBA8,
    };

    struct InternalData
    {
        Slice<u8> pixels;
        Vector2I size;
        ImageFormat format;
    } data;
    
    void init(const ResourceCreateInfo& info);
    void destroy();
    
    Error load(StringView file_path);
    void unload();

    Slice<u8> get_raw_pixels() const { return data.pixels; }
    Vector2I get_size() const { return data.size; }
    ImageFormat get_format() const { return data.format; }
};

