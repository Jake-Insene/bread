#include "graphics/render_target.h"

#include "graphics/graphics.h"

RenderTarget RenderTarget::create(Vector2I size)
{
    return RenderTarget
    {
        .render_target_id = Graphics::create_render_target(
            RenderTargetCreateInfo
            {
                .format = TEXTURE_FORMAT_RGBA8,
                .depth_stencil_format = TEXTURE_FORMAT_UNKNOWN,
                .size = size,
            }
        ),
    };
}

void RenderTarget::destroy()
{
    Graphics::destroy_render_target(render_target_id);
}

Vector2I RenderTarget::get_size() const
{
    return Graphics::render_target_get_size(render_target_id);
}

void RenderTarget::set_size(const Vector2I& new_size) const
{
    Graphics::render_target_set_size(render_target_id, new_size);
}
