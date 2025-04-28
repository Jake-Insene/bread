#pragma once
#include "core/header.h"

enum ResourceType
{
    RESOURCE_UNKNOWN = 0,
    RESOURCE_IMAGE,
    
    RESOURCE_TEXTURE_2D,
 };

struct Resource
{
    ResourceType type;
    String path;
    
    void destroy();
};
