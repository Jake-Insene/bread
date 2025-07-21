#include "resource/resource_manager.h"

#include "debug/debug.h"
#include "freetype/fttypes.h"
#include "fs/file.h"
#include "graphics/graphics.h"
#include "os/os.h"

#include <external/stb_image.h>


#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SYSTEM_H
#include FT_MODULE_H

FT_MemoryRec_ ft_memory_rec;

static inline void* _ft_alloc(FT_Memory, long size)
{
    return ResourceManager::get_allocator().alloc(size, 16).items;
}

static inline void* _ft_realloc(FT_Memory, long old_size, long new_size, void* mem)
{
    Slice<u8> old_mem = Slice((u8*)mem, old_size);
    if (ResourceManager::get_allocator().realloc(old_mem, new_size, 16))
    {
        return mem;
    }

    Slice<u8> new_mem = ResourceManager::get_allocator().alloc(new_size, 16);
    if (mem != nullptr)
    {
        mem::copy(new_mem, old_mem);
        ResourceManager::get_allocator().free(old_mem);
    }

    return new_mem.items;
}

static inline void _ft_free(FT_Memory, void* mem)
{
    if (mem == &ft_memory_rec)
        return;

    ResourceManager::get_allocator().free(Slice((u8*)mem, 1));
}

void ResourceManager::initialize(mem::Allocator& allocator)
{
    data.allocator = allocator;

    ft_memory_rec =
    {
        .user = nullptr,
        .alloc = _ft_alloc,
        .free = _ft_free,
        .realloc = _ft_realloc,
    };

    FailOn(OS::set_current_directory("assets") == false, "assets directory not found")
    
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
        return load_image(path);
    case RESOURCE_TEXTURE:
        break;
    case RESOURCE_TEXTURE_2D:
        return load_texture_2d(
            path,
            TextureLoadInfo
            {
                .type = TEXTURE_2D,
                .min_filter = TEXTURE_FILTER_NEAREST,
                .mag_filter = TEXTURE_FILTER_NEAREST,
            }
        );
        break;
    case RESOURCE_FONT:
        return load_font(path, Font::DefaultFontSize);
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

Image* ResourceManager::load_image(StringView path)
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
        {
            Fatal("Couldn't load the image '{}'", path);
        }

        image = create_resource<Image>();
        *image = tmp_image;
        image->path.set(path);
        data.resources.insert(path, (Resource*)image);
    }

    return image;
}


Texture2D* ResourceManager::load_texture_2d(StringView path, const TextureLoadInfo& load_info)
{
    Image* image = nullptr;
    if(data.resources.has(path))
    {
        image = (Image*)data.resources.get(path);
    }
    else
    {
        image = create_resource<Image>();
        image->path.set(path);
        data.resources.insert(path, (Resource*)image);
        
        if(!image->load(path))
        {
            Fatal("Couldn't load the image '{}'", path);
        }
    }
    
    Texture2D* tex = nullptr;
    if(data.cached_images.has(image))
    {
        tex = (Texture2D*)data.cached_images.get(image);
    }
    else
    {
        DebugInfo("Loading the texture '{}'...", path);
        tex = create_resource<Texture2D>();
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
        tex->texture_id = Graphics::texture_create(create_info);
        Graphics::texture_set_image(tex->texture_id, image);

        data.cached_images.insert(image, tex);
        DebugInfo("'{}' was loaded correctly.", path);
    }
    
    return tex;
}


Font* ResourceManager::load_font(StringView path, i32 font_size)
{
    Font* new_font = create_resource<Font>();
    new_font->load_from_file(path, font_size);

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

    SpriteAnimation* sprite_animation = create_resource<SpriteAnimation>();
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

    TileSet* tile_set = create_resource<TileSet>();
    data.resources.insert(name, tile_set);
    tile_set->path.set("local");
    tile_set->set_tile_size(tile_size);
    return tile_set;
}
