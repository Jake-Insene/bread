#include "resource/texture.h"

#include "resource/resource_manager.h"


void Texture::init()
{
    Resource::init(RESOURCE_TEXTURE);
}

void Texture::destroy()
{
    Resource::destroy();
    GPU::texture_destroy(texture_id);
}

Vector2I Texture::get_size() const
{
    return GPU::texture_get_size(texture_id);
}

void Texture2D::init()
{
    Resource::init(RESOURCE_TEXTURE_2D);
}

void Texture2D::destroy()
{
    Resource::destroy();
}

