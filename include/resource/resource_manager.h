#pragma once
#include "Collections/Error.hpp"
#include "Collections/HashMap.hpp"
#include "Collections/StringMap.hpp"
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

    Mem::Allocator& allocator;
    Collections::StringMap<ResourceAllocation> resources;

    [[nodiscard]] Mem::Allocator& get_allocator() const { return allocator; }

    ResourceManager(Mem::Allocator& allocator);
    ~ResourceManager();

    [[nodiscard]] Collections::Result<Resource*, Error> load_resource(ResourceType type,
        ResourceTypeSpecification specification, Collections::StringView path);

    [[nodiscard]] bool place_resource(Collections::StringView resource_name, Resource* resource);

    // Implementation
    template<typename T>
    requires(!Core::IsSame<Resource, T>)
    [[nodiscard]] T* _create_resource()
    {
        T* resource = get_allocator().object<T>(
            Resource::ResourceCreateInfo
            {
                .allocator = get_allocator(),
                .resource_type = T::Type,
            }
        );
        return resource;
    }

    [[nodiscard]] Collections::Result<Resource*, Error> _load_image(Collections::StringView path);
    [[nodiscard]] Collections::Result<Resource*, Error> _load_texture_2d(Collections::StringView path);
    [[nodiscard]] Collections::Result<Resource*, Error> _load_sound(Collections::StringView path);
    [[nodiscard]] Collections::Result<Resource*, Error> _load_font(Collections::StringView path);
};


