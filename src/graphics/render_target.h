#pragma once
#include "resource/resource_id.h"
#include "math/vec2.h"

struct [[nodiscard]] RenderTarget
{
    ResourceID render_target_id;
    
    static RenderTarget create(Vector2I size);
    
    void destroy();

    Vector2I get_size() const;
    void set_size(const Vector2I& new_size) const;
};
