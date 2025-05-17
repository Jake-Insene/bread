#include "io/texture.h"

#include "graphics/graphics.h"
#include "io/resource_manager.h"


void Texture::destroy()
{
    Resource::destroy();
}


Vector2I Texture::get_size() const
{
    return Graphics::texture_get_size(texture_id);
}

