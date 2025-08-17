#include "resource/resource_manager.h"

#include "debug/debug.h"
#include "freetype/fttypes.h"
#include "fs/file.h"
#include "graphics/graphics.h"
#include "os/os.h"

#include <external/stb_image.h>


void ResourceManager::initialize(mem::Allocator& allocator)
{
    data.allocator = allocator;

    stbi_set_flip_vertically_on_load(true);
    data.resources = StringMap<Resource*>::with_size(
        data.allocator, 4
    );
    
    data.cached_images = HashMap<Image*, Texture*>::with_size(
        data.allocator, 4
    );
}

void ResourceManager::shutdown()
{
    for(auto& it : data.resources)
    {
        switch (it.second->type)
        {
        case RESOURCE_IMAGE:
        {
            Image* image = (Image*)it.second;
            image->destroy();
        }
            break;
        case RESOURCE_SOUND:
        {
            Sound* sound = (Sound*)it.second;
            sound->destroy();
        }
            break;
        case RESOURCE_FONT:
            {
                Font* font = (Font*)it.second;
                font->destroy();
            }
            break;
        case RESOURCE_SPRITE_ANIMATION:
        {
            SpriteAnimation* sa = (SpriteAnimation*)it.second;
            sa->destroy();
        }
            break;
        case RESOURCE_TILE_SET:
        {
            TileSet* ts = (TileSet*)it.second;
            ts->destroy();
        }
        break;
        default:
            break;
        }
        
        data.allocator.free(
            mem::to_bytes(Slice<Resource>(it.second, 1))
        );
    }
    
    for(auto& it : data.cached_images)
    {
        if(it.second)
        {
            it.second->destroy();
            data.allocator.free(
                mem::to_bytes(Slice<Texture>(it.second, 1))
            );
        }
    }

    data.resources.destroy();
    data.cached_images.destroy();
}

Resource* ResourceManager::load_resource(ResourceType type,
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
                .type = TEXTURE_2D,
                .min_filter = TEXTURE_FILTER_NEAREST,
                .mag_filter = TEXTURE_FILTER_NEAREST,
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
        return nullptr;
    }
    break;
    default:
        return nullptr;
    }

    return nullptr;
}


bool ResourceManager::place_resource(StringView path, Resource* resource)
{
    if (data.resources.has(path))
        return false;

    data.resources.insert(path, resource);
    return true;
}

Image* ResourceManager::_load_image(StringView path)
{
    Image* image = nullptr;
    if (data.resources.has(path))
    {
        image = (Image*)data.resources.get(path);
    }
    else
    {
        Image tmp_image{};
        if (!tmp_image.load(path))
            return nullptr;

        image = _create_resource<Image>();
        *image = tmp_image;
        image->path.set(path);
        data.resources.insert(path, (Resource*)image);
    }

    return image;
}


Texture2D* ResourceManager::_load_texture_2d(StringView path, const TextureLoadInfo& load_info)
{
    Image* image = nullptr;
    if(data.resources.has(path))
    {
        image = (Image*)data.resources.get(path);
    }
    else
    {
        image = _create_resource<Image>();
        image->path.set(path);
        data.resources.insert(path, (Resource*)image);
        
        if (!image->load(path))
            return nullptr;
    }
    
    Texture2D* tex = nullptr;
    if(data.cached_images.has(image))
    {
        tex = (Texture2D*)data.cached_images.get(image);
    }
    else
    {
        RMDebugInfo("Loading the texture '{}'...", path);
        tex = _create_resource<Texture2D>();
        tex->path.set(path);
        
        TextureCreateInfo create_info =
        {
            .type = load_info.type,
            .format = image->format == Image::FORMAT_RGB8 ? TEXTURE_FORMAT_RGB8 : TEXTURE_FORMAT_RGBA8,
            .min_filter = load_info.min_filter,
            .mag_filter = load_info.mag_filter,
            .size = image->size,
            .pixels = image->pixels,
        };
        tex->texture_id = Graphics::create_texture(create_info);
        Graphics::texture_set_image(tex->texture_id, image);

        data.cached_images.insert(image, tex);
        RMDebugInfo("'{}' was loaded correctly.", path);
    }
    
    return tex;
}

Sound* ResourceManager::_load_sound(StringView path)
{
    if (data.resources.has(path))
    {
        return (Sound*)data.resources.get(path);
    }

    Sound* new_font = _create_resource<Sound>();
    new_font->load(path);

    data.resources.insert(path, new_font);
    return new_font;
}

Font* ResourceManager::_load_font(StringView path)
{
    if (data.resources.has(path))
    {
        return (Font*)data.resources.get(path);
    }

    Font* new_font = _create_resource<Font>();
    new_font->load_from_file(path);

    data.resources.insert(path, new_font);
    return new_font;
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
