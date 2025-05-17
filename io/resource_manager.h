#pragma once
#include "core/header.h"
#include "graphics/structs.h"
#include "io/image.h"
#include "io/texture.h"
#include "io/sprite_animation.h"
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

    [[nodiscard]] static mem::Allocator& get_allocator() { return data.allocator; }
    
    static void initialize(mem::Allocator& allocator);
    static void shutdown();
    
    [[nodiscard]] static Resource* load_resource(ResourceType type, 
        ResourceTypeSpecification specification, StringView path);
    [[nodiscard]] static Image* load_image(StringView path);
    [[nodiscard]] static Texture2D* load_texture_2d(StringView path, const TextureLoadInfo& load_info);

    [[nodiscard]] static SpriteAnimation* create_sprite_animation(StringView name);
};

// Try to load the resource of the given type, can return nullptr
template<typename T>
T* GetResource(StringView path)
{
    return reinterpret_cast<T*>(ResourceManager::load_resource(T::Type, T::Specification, path));
};

