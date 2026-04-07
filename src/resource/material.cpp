#include "resource/material.h"

#include "io/file.h"
#include "resource/resource_manager.h"
#include "resource/resource_manager_internal.h"



Error Material::load_from_file(StringView file_path, StringView defines)
{
    Unused(file_path, defines);
    return MakeError(ErrorCode::ResourceNotFound);
}


