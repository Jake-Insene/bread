#pragma once
#include "graphics/graphics_types.h"
#include "math/vec2.h"


/*
    Contains a texture to render to.
*/
struct [[nodiscard]] RenderTarget
{
    RenderTargetID render_target_id;
    
    static RenderTarget get_main_render_target();
    static RenderTarget create(Vector2I size);
    
    void destroy();

    void set_size(const Vector2I& new_size) const;
    Vector2I get_size() const;
};
