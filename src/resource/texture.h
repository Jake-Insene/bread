#pragma once
#include "resource/resource.h"
#include "graphics/graphics_types.h"
#include "math/vec2.h"


// A group of pixels allocated in the gpu.
struct Texture : Resource
{
    RESOURCE(RESOURCE_TEXTURE, LoadFromAssets, ResourceExtensions("png"));

    TextureID texture_id;
    
    void init();
    void destroy();
    
    [[nodiscard]] Vector2I get_size() const;
};

// Same as Texture but specialized for 2D.
struct Texture2D : Texture
{
    RESOURCE(RESOURCE_TEXTURE_2D, LoadFromAssets, ResourceExtensions("png"));

    void init();
    void destroy();
};

