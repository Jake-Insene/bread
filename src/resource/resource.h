#pragma once
#include "collections/string.h"


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
    static constexpr bool IsResource = true;

    RESOURCE(
        RESOURCE_UNKNOWN,
        NoResourceFlags, 
        ResourceExtensions(""));
    
    ResourceType type;
    String path;
    
    void init(ResourceType resource_type);
    void destroy();
};

template<typename T>
concept IsResourceBase = requires
{
    T::IsResource;
};
