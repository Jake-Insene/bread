#pragma once
#include "collections/error.h"
#include "collections/result.h"
#include "collections/string.h"
#include "collections/string_view.h"


enum ResourceType
{
    RESOURCE_UNKNOWN = 0,
    RESOURCE_IMAGE,
    
    RESOURCE_TEXTURE,
    RESOURCE_TEXTURE_2D,
    RESOURCE_SOUND,
    RESOURCE_FONT,
    RESOURCE_SPRITE_ANIMATION,
    RESOURCE_TILE_SET,
    RESOURCE_MATERIAL,
 };

struct ResourceTypeSpecification
{
    bool LoadFromAssets = false;
    StringView Extensions = "";
};

enum
{
    NoResourceFlags = 0,
    LoadFromAssets = 1,
};

static constexpr ResourceTypeSpecification _construct_from_flags(usize flags, StringView extensions)
{
    return ResourceTypeSpecification
    {
        .LoadFromAssets = bool(flags & LoadFromAssets),
        .Extensions = extensions,
    };
}

#define RESOURCE(resource_type, flags, extensions) \
    static constexpr ResourceTypeSpecification Specification = _construct_from_flags(flags, extensions);\
    static constexpr ResourceType Type = resource_type;\

#define ResourceExtensions(extensions) extensions


/*
* A 'Resource' represents a collection of data that can be reused across the application.
*/
struct Resource
{
    RESOURCE(
        RESOURCE_UNKNOWN,
        NoResourceFlags, 
        ResourceExtensions(""))

    
    static Result<Resource*, Error> _load_resource(ResourceType type, ResourceTypeSpecification spec, StringView path);

    /*
    * Try to load the resource of the given type, can return nullptr
    */
    template<typename T>
        requires(!IsSame<Resource, T> && IsBaseOf<Resource, T>)
    [[nodiscard]] static Result<T*, Error> try_load(StringView path)
    {
        Result<Resource*, Error> resource = _load_resource(T::Type, T::Specification, path);
        if (resource)
        {
            return reinterpret_cast<T*>(resource.value());
        }

        return resource.error();
    };


    /*
    * Load the resource of the given type, can return nullptr.
    */
    template<typename T>
        requires(!IsSame<Resource, T> && IsBaseOf<Resource, T>)
    [[nodiscard]] static T* load(StringView path)
    {
        return reinterpret_cast<T*>(_load_resource(T::Type, T::Specification, path).value());
    };
    
    ResourceType type;
    String path;
    
    void init(ResourceType resource_type);
    void destroy();
};


