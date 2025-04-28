#pragma once
#include "core/header.h"
#include "graphics/structs.h"
#include "io/image.h"
#include "io/texture.h"
#include "mem/allocator.h"


struct ResourceManager
{
    // As everything in a struct is public we need to hide data
    // that should not be modified/access directly, this also
    // resolve some namespace problems.
    struct InternalData
    {
        mem::Allocator allocator;
        StringMap<Resource*> resources;
        HashMap<Image*, Texture*> cached_images;
    };

    static inline InternalData data;
    
    // Default resources
    static inline Texture white_texture;
    
    static void initialize(mem::Allocator& allocator);
    static void shutdown();
    
    [[nodiscard]] static Texture2D* load_texture_2d(StringView path, const TextureLoadInfo& load_info);

    [[nodiscard]] static mem::Allocator& get_allocator() { return data.allocator; }
};

// Try to load the resource of the given type, can return nullptr
template<typename T>
T* GetResource(StringView path)
{
    return Texture2D::load_from_file(path);
};

