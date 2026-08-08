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
 };

struct ResourceTypeSpecification
{
    bool LoadFromAssets = false;
    StringView Extensions = "";
};

enum class ResourceFlags : u32
{
    NoResourceFlags = 0,
    LoadFromAssets = 1,
};
EnableBitOp(ResourceFlags)

static constexpr ResourceTypeSpecification _construct_from_flags(ResourceFlags flags, StringView extensions)
{
    return ResourceTypeSpecification
    {
        .LoadFromAssets = Core::HasValue(flags & ResourceFlags::LoadFromAssets),
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
    DisableCopy(Resource);
    DisableMove(Resource);
    
    RESOURCE(
        RESOURCE_UNKNOWN,
        ResourceFlags::NoResourceFlags, 
        ResourceExtensions(""))

    struct ResourceCreateInfo
    {
        Mem::Allocator& allocator;
        ResourceType resource_type;
    };

    
    static Result<Resource*, Error> _load_resource(ResourceType type, ResourceTypeSpecification spec, StringView path);

    /*
    * Try to load the resource of the given type, can return nullptr
    */
    template<typename T>
    requires(!Core::IsSame<Resource, T> && Core::IsBaseOf<Resource, T>)
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
    requires(!Core::IsSame<Resource, T> && Core::IsBaseOf<Resource, T>)
    [[nodiscard]] static T* load(StringView path)
    {
        return reinterpret_cast<T*>(_load_resource(T::Type, T::Specification, path).value());
    };
    
    Mem::Allocator& allocator;
    ResourceType type;
    String path;
    
    Resource(const ResourceCreateInfo& info);
    virtual ~Resource();
};


