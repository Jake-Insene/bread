#pragma once
#include "graphics/gpu_resource_types.h"
#include "math/vec2.h"
#include "resource/resource.h"
#include "resource/image.h"



// A group of pixels allocated in the gpu.
struct Texture : Resource
{
    RESOURCE(RESOURCE_TEXTURE, ResourceFlags::LoadFromAssets, ResourceExtensions("png"))

    Graphics::GPUTextureID texture_ref;
    Vector2I size;
    
    void init(const ResourceCreateInfo& info);
    void destroy();
    
    [[nodiscard]] Vector2I get_size() const;
};

// Same as Texture but specialized for 2D.
struct Texture2D : Texture
{
    RESOURCE(RESOURCE_TEXTURE_2D, ResourceFlags::LoadFromAssets, ResourceExtensions("png"))

    Error load_from_path(StringView file_path);
    Error load_from_image(Image* image);
    Error load_from_raw(Image::ImageFormat image_format, const Vector2I& image_size, const Slice<u8>& pixels);
};

