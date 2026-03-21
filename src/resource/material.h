#pragma once
#include "collections/string_view.h"
#include "resource/resource.h"


struct Material : Resource
{
    RESOURCE(RESOURCE_MATERIAL, LoadFromAssets, ResourceExtensions("shader"))

    void init();
    void destroy();

    Error load_from_file(StringView file_path, StringView defines);
};