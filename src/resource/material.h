#pragma once
#include "collections/string_view.h"
#include "render/material_manager.h"
#include "resource/resource.h"


struct Material : Resource
{
    RESOURCE(RESOURCE_MATERIAL, LoadFromAssets, ResourceExtensions("shader"));
    MaterialManager::MaterialID material_id;

    void init();
    void destroy();

    Error load_from_file(StringView file_path, StringView defines);
};