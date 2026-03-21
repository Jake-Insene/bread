#include "resource/material.h"

#include "io/file.h"
#include "resource/resource_manager.h"
#include "resource/resource_manager_internal.h"


void Material::init()
{
	Resource::init(RESOURCE_MATERIAL);
}

void Material::destroy()
{
    Resource::destroy();
}

Error Material::load_from_file(StringView file_path, StringView defines)
{
    Unused(file_path, defines);
    return MakeError(ErrorCode::ResourceNotFound);
}


