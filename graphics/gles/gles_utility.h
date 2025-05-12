#pragma once
#include "math/vec2.h"
#include "core/string.h"

namespace gles
{
    
    [[nodiscard]] u32 compile_program(StringView program_path, StringView defines);

}
