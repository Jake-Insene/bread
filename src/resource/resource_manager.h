#pragma once
#include "collections/hash_map.h"
#include "collections/string_map.h"
#include "graphics/structs.h"
#include "mem/allocator.h"
#include "resource/font.h"
#include "resource/image.h"
#include "resource/texture.h"
#include "resource/sprite_animation.h"
#include "resource/tile_set.h"


struct ResourceManager
{
    static constexpr usize DefaultFontSize = 32;

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

    [[nodiscard]] static mem::Allocator& get_allocator() { return data.allocator; }
    
    template<typename T>
    [[nodiscard]] static T* create_resource()
    {
        static_assert(!IsSame<T, Resource>, "Resource is not allowed");
        T* resource = get_allocator().object<T>();
        resource->init();
        return resource;
    }

    static void initialize(mem::Allocator& allocator);
    static void shutdown();

    [[nodiscard]] static Resource* load_resource(ResourceType type, 
        ResourceTypeSpecification specification, StringView path);

    [[nodiscard]] bool place_resource(StringView path, Resource* resource);

    [[nodiscard]] static Image* load_image(StringView path);
    [[nodiscard]] static Texture2D* load_texture_2d(StringView path, const TextureLoadInfo& load_info);
    [[nodiscard]] static Font* load_font(StringView path, i32 font_size);

    [[nodiscard]] static SpriteAnimation* create_sprite_animation(StringView name);
    [[nodiscard]] static TileSet* create_tile_set(StringView name, Vector2I tile_size);
};

// Try to load the resource of the given type, can return nullptr
template<typename T>
T* GetResource(StringView path)
{
    static_assert(!IsSame<T, Resource>, "Resource is not allowed");
    return reinterpret_cast<T*>(ResourceManager::load_resource(T::Type, T::Specification, path));
};

