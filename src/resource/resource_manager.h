#pragma once
#include "collections/error.h"
#include "collections/hash_map.h"
#include "collections/string_map.h"
#include "gpu/gpu.h"
#include "mem/allocator.h"
#include "resource/resource.h"
#include "systems/system.h"


struct Image;
struct SpriteAnimation;
struct TileSet;
struct Texture;


struct ResourceManager : System<ResourceManager>
{
    static constexpr SystemDependency Dependencies[] =
    {
        SystemDependency::of("RenderDevice")
    };

    static constexpr StringView _name = "ResourceManager";
    static constexpr SystemInfo get_system_info()
    {
        return System::get_system_info_with_name(_name);
    }

    static constexpr usize DefaultFontSize = 32;

    struct TextureLoadInfo
    {
        GPU::TextureType type;
    };

    mem::Allocator allocator;
    StringMap<Resource*> resources;
    HashMap<Image*, Texture*> cached_images;

    [[nodiscard]] mem::Allocator& get_allocator() { return allocator; }

    void initialize(const SystemInitializeInfo& info);
    void shutdown();

    void on_event(const InputEvent&) {}

    [[nodiscard]] Result<Resource*, Error> load_resource(ResourceType type,
        ResourceTypeSpecification specification, StringView path);

    [[nodiscard]] bool place_resource(StringView resource_name, Resource* resource);

    [[nodiscard]] SpriteAnimation* create_sprite_animation(StringView name);
    [[nodiscard]] TileSet* create_tile_set(StringView name, Vector2I tile_size);
    
    // Implementation
    template<typename T>
        requires(!IsSame<Resource, T>)
    [[nodiscard]] T* _create_resource()
    {
        T* resource = get_allocator().object<T>();
        resource->init();
        return resource;
    }

    [[nodiscard]] Result<Resource*, Error> _load_image(StringView path);
    [[nodiscard]] Result<Resource*, Error> _load_texture_2d(StringView path, const TextureLoadInfo& load_info);
    [[nodiscard]] Result<Resource*, Error> _load_sound(StringView path);
    [[nodiscard]] Result<Resource*, Error> _load_font(StringView path);
    [[nodiscard]] Result<Resource*, Error> _load_material(StringView path);
};


