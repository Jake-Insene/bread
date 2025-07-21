#pragma once
#include "math/vec2.h"
#include "collections/string_view.h"

namespace gles
{
    
    [[nodiscard]] u32 compile_program(StringView program_path, StringView defines);

}
