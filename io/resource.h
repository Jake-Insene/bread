#pragma once
#include "core/header.h"


struct ResourceTypeSpecification
{
    bool LoadFromAssets = false;
    StringView Extensions = "";
};

#define RESOURCE(ResType, ...) \
    static constexpr ResourceTypeSpecification Specification = {__VA_ARGS__};\
    static constexpr ResourceType Type = ResType\

enum ResourceType
{
    RESOURCE_UNKNOWN = 0,
    RESOURCE_IMAGE,
    
    RESOURCE_TEXTURE,
    RESOURCE_TEXTURE_2D,
    RESOURCE_SPRITE_ANIMATION,
    RESOURCE_TILE_SET,
 };

// A 'Resource' represents a collection of data that can be reused across the application.
struct Resource
{
    RESOURCE(RESOURCE_UNKNOWN, .LoadFromAssets = false, .Extensions = "");
    ResourceType type;
    String path;
    
    void destroy();
};
