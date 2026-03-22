#include "resource/resource_manager.h"

#include "engine/engine.h"
#include "io/file.h"
#include "resource/resource_manager_internal.h"
#include "resource/font.h"
#include "resource/image.h"
#include "resource/material.h"
#include "resource/texture.h"
#include "resource/sound.h"
#include "resource/sprite_animation.h"
#include "resource/tile_set.h"
#include "render/render_device.h"

#include <external/stb_image.h>


void ResourceManager::initialize(mem::Allocator& allocator)
{
    data.allocator = allocator;

    stbi_set_flip_vertically_on_load(true);
    data.resources = StringMap<Resource*>::with_size(
        data.allocator, 128
    );
    
    data.cached_images = HashMap<Image*, Texture*>::with_size(
        data.allocator, 128
    );
}

void ResourceManager::shutdown()
{
    for(auto& it : data.resources.iter())
    {
        switch (it.second->type)
        {
        case RESOURCE_IMAGE:
        {
            Image* image = reinterpret_cast<Image*>(it.second);
            image->destroy();
        }
            break;
        case RESOURCE_SOUND:
        {
            Sound* sound = reinterpret_cast<Sound*>(it.second);
            sound->destroy();
        }
            break;
        case RESOURCE_FONT:
            {
                Font* font = reinterpret_cast<Font*>(it.second);
                font->destroy();
            }
            break;
        case RESOURCE_SPRITE_ANIMATION:
        {
            SpriteAnimation* sa = reinterpret_cast<SpriteAnimation*>(it.second);
            sa->destroy();
        }
            break;
        case RESOURCE_TILE_SET:
        {
            TileSet* ts = reinterpret_cast<TileSet*>(it.second);
            ts->destroy();
        }
        break;
        case RESOURCE_MATERIAL:
        {
            Material* ma = reinterpret_cast<Material*>(it.second);
            ma->destroy();
        }
            break;
        default:
            break;
        }
        
        data.allocator.free(
            mem::to_bytes(Slice<Resource>(it.second, 1))
        );
    }
    
    for(auto& [image, texture] : data.cached_images.iter())
    {
        if(texture)
        {
            texture->destroy();
            data.allocator.free(
                mem::to_bytes(Slice<Texture>(texture, 1))
            );
        }
    }

    data.resources.destroy();
    data.cached_images.destroy();
}

Result<Resource*, Error> ResourceManager::load_resource(ResourceType type,
    ResourceTypeSpecification, StringView path)
{
    switch (type)
    {
    case RESOURCE_IMAGE:
        return _load_image(path);
    case RESOURCE_TEXTURE:
        break;
    case RESOURCE_TEXTURE_2D:
        return _load_texture_2d(
            path,
            TextureLoadInfo
            {
                .type = GPU::TextureType::Texture2D,
            }
        );
        break;
    case RESOURCE_SOUND:
        return _load_sound(path);
        break;
    case RESOURCE_FONT:
        return _load_font(path);
    case RESOURCE_SPRITE_ANIMATION:
    case RESOURCE_TILE_SET:
    {
        if (data.resources.has(path))
        {
            return data.resources.get(path);
        }
        return MakeError(ErrorCode::ResourceNotFound);
    }
    break;
    case RESOURCE_MATERIAL:
    {
        return _load_material(path);
    }
        break;
    default:
        break;
    }

    return MakeError(ErrorCode::InvalidResourceType);
}


bool ResourceManager::place_resource(StringView resource_name, Resource* resource)
{
    if (data.resources.has(resource_name))
        return false;

    data.resources.insert(resource_name, resource);
    return true;
}

SpriteAnimation* ResourceManager::create_sprite_animation(StringView name)
{
    if (data.resources.has(name))
    {
        FailOn(true, "SpriteAnimation already create");
        return nullptr;
    }

    SpriteAnimation* sprite_animation = _create_resource<SpriteAnimation>();
    data.resources.insert(name, sprite_animation);

    sprite_animation->path.set("local");
    return sprite_animation;
}

TileSet* ResourceManager::create_tile_set(StringView name, Vector2I tile_size)
{
    if (data.resources.has(name))
    {
        FailOn(true, "TileSet already create");
        return nullptr;
    }

    TileSet* tile_set = _create_resource<TileSet>();
    data.resources.insert(name, tile_set);
    tile_set->path.set("local");
    tile_set->set_tile_size(tile_size);
    return tile_set;
}

Result<Resource*, Error> ResourceManager::_load_image(StringView path)
{
    Image* image = nullptr;
    if (data.resources.has(path))
    {
        image = reinterpret_cast<Image*>(data.resources.get(path));
    }
    else
    {
        image = _create_resource<Image>();

        Error load_result = image->load(path);
        if (!load_result)
        {
            get_allocator().free(mem::to_bytes(Slice(image, 1)));
            return load_result;
        }

        image->path.set(path);
        (void)place_resource(path, image);
    }

    return image;
}


Result<Resource*, Error> ResourceManager::_load_texture_2d(StringView path, const TextureLoadInfo& load_info)
{
    Image* image = nullptr;
    if(data.resources.has(path))
    {
        image = reinterpret_cast<Image*>(data.resources.get(path));
    }
    else
    {
        image = _create_resource<Image>();
        image->path.set(path);
        (void)place_resource(path, image);
        
        Error load_result = image->load(path);
        if (!load_result)
            return load_result;
    }
    
    Texture2D* tex = nullptr;
    if(data.cached_images.has(image))
    {
        tex = reinterpret_cast<Texture2D*>(data.cached_images.get(image));
    }
    else
    {
        RMDebugInfo("Loading texture '{}'...", path);
        tex = _create_resource<Texture2D>();
        tex->path.set(path);
        
        GPUTextureResourceCreateInfo create_info =
        {
            .type = load_info.type,
            .format = image->format == Image::FORMAT_RGB8 ? GPU::TextureFormat::RGB8Srgb : GPU::TextureFormat::RGBA8Srgb,
            .extent = Vector3U(image->size.width, image->size.height, 1),
            .pixels = image->pixels,
        };
        
        tex->texture_ref = Engine::get_system_manager().get_system<RenderDevice>()->get_resource_manager().create_texture(create_info);
        tex->size = image->size;
        data.cached_images.insert(image, tex);
    }
    
    return tex;
}

Result<Resource*, Error> ResourceManager::_load_sound(StringView path)
{
    if (data.resources.has(path))
    {
        return reinterpret_cast<Sound*>(data.resources.get(path));
    }

    Sound* new_sound = _create_resource<Sound>();
    Error load_result = new_sound->load(path);
    if (!load_result)
    {
        return load_result;
    }

    (void)place_resource(path, new_sound);
    return new_sound;
}

Result<Resource*, Error> ResourceManager::_load_font(StringView path)
{
    if (data.resources.has(path))
    {
        return reinterpret_cast<Font*>(data.resources.get(path));
    }

    Font* new_font = _create_resource<Font>();
    Error load_result = new_font->load(path);
    if (!load_result)
    {
        return load_result;
    }

    (void)place_resource(path, new_font);
    return new_font;
}

Result<Resource*, Error> ResourceManager::_load_material(StringView path)
{
    if (data.resources.has(path))
    {
        return reinterpret_cast<Material*>(data.resources.get(path));
    }

    Material* new_material = _create_resource<Material>();
    Error load_result = new_material->load_from_file(path, "");
    if (!load_result)
    {
        return load_result;
    }

    (void)place_resource(path, new_material);
    return new_material;
}

