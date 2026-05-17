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
#include "render_device/render_device.h"

#include <external/stb_image.h>


void ResourceManager::initialize(const ResourceManagerCreateInfo& info)
{
    allocator = info.allocator;

    stbi_set_flip_vertically_on_load(true);
    resources = StringMap<ResourceAllocation>::with_size(
        allocator, 128
    );
    
    cached_images = HashMap<Image*, Texture*>::with_size(
        allocator, 128
    );
}

void ResourceManager::shutdown()
{
    for(auto& it : resources.iter())
    {
        it.second.destroy(it.second.resource);
        allocator->free(
            mem::to_bytes(Slice<Resource>(it.second.resource, 1))
        );
    }
    
    for(auto& [image, texture] : cached_images.iter())
    {
        if(texture)
        {
            texture->destroy();
            allocator->free(
                mem::to_bytes(Slice<Texture>(texture, 1))
            );
        }
    }

    resources.destroy();
    cached_images.destroy();
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
        if (resources.has(path))
        {
            return resources.get(path).resource;
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


bool ResourceManager::place_resource(StringView resource_name, DestroyResourceFn destroy, Resource* resource)
{
    if (resources.has(resource_name))
        return false;

    resources.insert(
        resource_name,
        {
            .destroy = destroy,
            .resource = resource,
        }
    );
    return true;
}

SpriteAnimation* ResourceManager::create_sprite_animation(StringView name)
{
    if (resources.has(name))
    {
        FailOn(true, "SpriteAnimation already create");
        return nullptr;
    }

    SpriteAnimation* sprite_animation = _create_resource<SpriteAnimation>();
    (void)place_resource(
        name,
        [](Resource* resource){ reinterpret_cast<SpriteAnimation*>(resource)->destroy(); },
        sprite_animation
    );

    sprite_animation->path.set("local");
    return sprite_animation;
}

TileSet* ResourceManager::create_tile_set(StringView name, Vector2I tile_size)
{
    if (resources.has(name))
    {
        FailOn(true, "TileSet already create");
        return nullptr;
    }

    TileSet* tile_set = _create_resource<TileSet>();
    (void)place_resource(
        name,
        [](Resource* resource){ reinterpret_cast<TileSet*>(resource)->destroy(); },
        tile_set
    );
    
    tile_set->path.set("local");
    tile_set->set_tile_size(tile_size);
    return tile_set;
}

Result<Resource*, Error> ResourceManager::_load_image(StringView path)
{
    Image* image = nullptr;
    if (resources.has(path))
    {
        image = reinterpret_cast<Image*>(resources.get(path).resource);
    }
    else
    {
        image = _create_resource<Image>();

        Error load_result = image->load(path);
        if (!load_result)
        {
            get_allocator()->free(mem::to_bytes(Slice(image, 1)));
            return load_result;
        }

        image->path.set(path);
        (void)place_resource(
            path,
            [](Resource* resource){ reinterpret_cast<Image*>(resource)->destroy(); },
            image
        );
    }

    return image;
}


Result<Resource*, Error> ResourceManager::_load_texture_2d(StringView path, const TextureLoadInfo& load_info)
{
    Image* image = nullptr;
    if(resources.has(path))
    {
        image = reinterpret_cast<Image*>(resources.get(path).resource);
    }
    else
    {
        image = _create_resource<Image>();
        image->path.set(path);
        (void)place_resource(
            path,
            [](Resource* resource){ reinterpret_cast<Image*>(resource)->destroy(); },
            image
        );
        
        Error load_result = image->load(path);
        if (!load_result)
            return load_result;
    }
    
    Texture2D* tex = nullptr;
    if(cached_images.has(image))
    {
        tex = reinterpret_cast<Texture2D*>(cached_images.get(image));
    }
    else
    {
        RMDebugInfo("Loading texture '{}'...", path);
        tex = _create_resource<Texture2D>();
        tex->path.set(path);
        
        GPUResourceManager::TextureAllocateInfo create_info =
        {
            .type = load_info.type,
            .format = image->get_format() == Image::ImageFormat::RGB8 ? GPU::TextureFormat::RGB8Srgb : GPU::TextureFormat::RGBA8Srgb,
            .extent = Vector3U(image->get_size().width, image->get_size().height, 1),
            .pixels = image->get_raw_pixels(),
        };
        
        tex->texture_ref = Engine::get_render_device()->get_gpu_resource_manager()->create_texture(create_info);
        tex->size = image->get_size();
        cached_images.insert(image, tex);
    }
    
    return tex;
}

Result<Resource*, Error> ResourceManager::_load_sound(StringView path)
{
    if (resources.has(path))
    {
        return reinterpret_cast<Sound*>(resources.get(path).resource);
    }

    Sound* new_sound = _create_resource<Sound>();
    Error load_result = new_sound->load(path);
    if (!load_result)
    {
        return load_result;
    }

    (void)place_resource(
        path,
        [](Resource* resource){ reinterpret_cast<Sound*>(resource)->destroy(); },
        new_sound
    );
    return new_sound;
}

Result<Resource*, Error> ResourceManager::_load_font(StringView path)
{
    if (resources.has(path))
    {
        return reinterpret_cast<Font*>(resources.get(path).resource);
    }

    Font* new_font = _create_resource<Font>();
    Error load_result = new_font->load(path);
    if (!load_result)
    {
        return load_result;
    }

    (void)place_resource(
        path,
        [](Resource* resource){ reinterpret_cast<Font*>(resource)->destroy(); },
        new_font
    );
    return new_font;
}

Result<Resource*, Error> ResourceManager::_load_material(StringView path)
{
    if (resources.has(path))
    {
        return reinterpret_cast<Material*>(resources.get(path).resource);
    }

    Material* new_material = _create_resource<Material>();
    Error load_result = new_material->load_from_file(path, "");
    if (!load_result)
    {
        return load_result;
    }

    (void)place_resource(
        path,
        [](Resource* resource){ reinterpret_cast<Material*>(resource)->destroy(); },
        new_material
    );
    return new_material;
}

