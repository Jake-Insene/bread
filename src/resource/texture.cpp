#include "resource/texture.h"

#include "engine/engine.h"
#include "resource/resource_manager.h"
#include "render/render_device.h"


void Texture::init()
{
    Resource::init(RESOURCE_TEXTURE);
    texture_ref = GPUTextureID::invalid();
    size = Vector2I();
}

void Texture::destroy()
{
    Resource::destroy();
    Engine::get_system_manager().get_system<RenderDevice>()->get_resource_manager().destroy_texture(texture_ref);
}

Vector2I Texture::get_size() const
{
    return size;
}

void Texture2D::init()
{
    Resource::init(RESOURCE_TEXTURE_2D);
}

void Texture2D::destroy()
{
    Resource::destroy();
}

