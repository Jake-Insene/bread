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

Texture2D* Texture2D::load_from_file(StringView path)
{
    return ResourceManager::load_texture_2d(
        path,
        TextureLoadInfo
        {
            .type = TEXTURE_2D,
            .min_filter = TEXTURE_FILTER_NEAREST,
            .mag_filter = TEXTURE_FILTER_NEAREST,
        }
    );
}
