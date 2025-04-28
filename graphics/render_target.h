#pragma once
#include "io/resource_id.h"
#include "math/vec2.h"

struct [[nodiscard]] RenderTarget
{
    ResourceID render_target_id;
    
    static RenderTarget create(Vector2I size);
    
    Vector2I get_size() const;
};
