#pragma once
#include "collections/error.h"
#include "collections/hash_map.h"
#include "collections/string_map.h"
#include "resource/resource.h"


struct Image;
struct SpriteAnimation;
struct TileSet;
struct Texture;


struct ResourceManager
{
    DisableCopy(ResourceManager);
    DisableMove(ResourceManager);

    static constexpr usize DefaultFontSize = 32;
    
    struct ResourceAllocation
    {
        Resource* resource;
    };

    Mem::Allocator* allocator;
    StringMap<ResourceAllocation> resources;

    [[nodiscard]] Mem::Allocator* get_allocator() const { return allocator; }

    ResourceManager(Mem::Allocator* allocator);
    ~ResourceManager();

    [[nodiscard]] Result<Resource*, Error> load_resource(ResourceType type,
        ResourceTypeSpecification specification, StringView path);

    [[nodiscard]] bool place_resource(StringView resource_name, Resource* resource);

    // Implementation
    template<typename T>
    requires(!IsSame<Resource, T>)
    [[nodiscard]] T* _create_resource()
    {
        T* resource = get_allocator()->object<T>(
            Resource::ResourceCreateInfo
            {
                .allocator = get_allocator(),
                .resource_type = T::Type,
            }
        );
        return resource;
    }

    [[nodiscard]] Result<Resource*, Error> _load_image(StringView path);
    [[nodiscard]] Result<Resource*, Error> _load_texture_2d(StringView path);
    [[nodiscard]] Result<Resource*, Error> _load_sound(StringView path);
    [[nodiscard]] Result<Resource*, Error> _load_font(StringView path);
};


