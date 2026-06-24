#pragma once
#include "graphics/gpu_resource_types.h"
#include "resource/resource.h"
#include "math/vec2.h"


// A group of pixels allocated in the gpu.
struct Texture : Resource
{
    RESOURCE(RESOURCE_TEXTURE, LoadFromAssets, ResourceExtensions("png"))

    GPUTextureID texture_ref;
    Vector2I size;
    
    void init(const ResourceCreateInfo& info);
    void destroy();
    
    [[nodiscard]] Vector2I get_size() const;
};

// Same as Texture but specialized for 2D.
struct Texture2D : Texture
{
    RESOURCE(RESOURCE_TEXTURE_2D, LoadFromAssets, ResourceExtensions("png"))
};

