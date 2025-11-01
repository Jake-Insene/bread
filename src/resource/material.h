#pragma once
#include "collections/string_view.h"
#include "resource/resource.h"
#include "graphics/graphics_types.h"


struct Material : Resource
{
    RESOURCE(RESOURCE_MATERIAL, LoadFromAssets, ResourceExtensions("shader"));
    MaterialID material_id;

    void init();
    void destroy();

    Error load_from_file(StringView path, StringView defines);
};