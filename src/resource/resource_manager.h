#pragma once
#include "collections/error.h"
#include "collections/hash_map.h"
#include "collections/string_map.h"
#include "graphics/structs.h"
#include "mem/allocator.h"
#include "resource/resource.h"


struct Image;
struct SpriteAnimation;
struct TileSet;
struct Texture;


struct ResourceManager
{
    static constexpr usize DefaultFontSize = 32;

    struct InternalData
    {
        mem::Allocator allocator;
        StringMap<Resource*> resources;
        HashMap<Image*, Texture*> cached_images;
    };

    static inline InternalData data;

    [[nodiscard]] static mem::Allocator& get_allocator() { return data.allocator; }

    static void initialize(mem::Allocator& allocator);
    static void shutdown();

    [[nodiscard]] static Result<Resource*, Error> load_resource(ResourceType type,
        ResourceTypeSpecification specification, StringView path);

    [[nodiscard]] static bool place_resource(StringView resource_name, Resource* resource);

    [[nodiscard]] static SpriteAnimation* create_sprite_animation(StringView name);
    [[nodiscard]] static TileSet* create_tile_set(StringView name, Vector2I tile_size);
    
    // Implementation
    template<typename T>
        requires(!IsSame<Resource, T>)
    [[nodiscard]] static T* _create_resource()
    {
        T* resource = get_allocator().object<T>();
        resource->init();
        return resource;
    }

    [[nodiscard]] static Result<Resource*, Error> _load_image(StringView path);
    [[nodiscard]] static Result<Resource*, Error> _load_texture_2d(StringView path, const TextureLoadInfo& load_info);
    [[nodiscard]] static Result<Resource*, Error> _load_sound(StringView path);
    [[nodiscard]] static Result<Resource*, Error> _load_font(StringView path);
    [[nodiscard]] static Result<Resource*, Error> _load_material(StringView path);
};


