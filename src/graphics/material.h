#pragma once
#include "collections/string_view.h"
#include "graphics/enums.h"
#include "graphics/graphics_types.h"


struct [[nodiscard]] Material
{
    MaterialID material_id = MaterialID();

    static Material create();

    void destroy();

    void compile_from_file(StringView path, StringView defines);
    void compile_from_source(StringView source, StringView defines);
};