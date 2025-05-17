#pragma once
#include "io/resource.h"
#include "io/resource_id.h"
#include "graphics/enums.h"
#include "math/vec2.h"

// A group of pixels allocated in the gpu.
struct Texture : Resource
{
    RESOURCE(RESOURCE_TEXTURE, .LoadFromAssets = true, .Extensions = "png");
    ResourceID texture_id;
    
    void destroy();
    
    [[nodiscard]] Vector2I get_size() const;
};

// Same as Texture but specialized for 2D.
struct Texture2D : Texture
{
    RESOURCE(RESOURCE_TEXTURE_2D, .LoadFromAssets = true, .Extensions = "png");
};

