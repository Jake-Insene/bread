#include "io/resource_manager.h"

#include "debug/debug.h"
#include "graphics/graphics.h"
#include "os/os.h"

#include <external/stb_image.h>


void ResourceManager::initialize(mem::Allocator& allocator)
{
    data = {};
    data.allocator = allocator;

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
        case RESOURCE_SPRITE_ANIMATION:
        {
            SpriteAnimation* sa = (SpriteAnimation*)it.second;
            sa->destroy();
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
        return ResourceManager::load_texture_2d(
            path,
            TextureLoadInfo
            {
                .type = TEXTURE_2D,
                .min_filter = TEXTURE_FILTER_NEAREST,
                .mag_filter = TEXTURE_FILTER_NEAREST,
            }
        );
    case RESOURCE_SPRITE_ANIMATION:
    {
        if (data.resources.has(path))
        {
            return data.resources.get(path);
        }
        return nullptr;
    }
    default:
        return nullptr;
    }

    return nullptr;
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
            Fatal("Couldn't load the image '%.*s'", (i32)path.len, path.items);
        }

        image = get_allocator().object<Image>();
        *image = tmp_image;
        image->type = RESOURCE_IMAGE;
        image->path = String::from_chars(get_allocator(), path);
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
        image = get_allocator().object<Image>();

        image->type = RESOURCE_IMAGE;
        image->path = String::from_chars(get_allocator(), path);
        data.resources.insert(path, (Resource*)image);
        
        if(!image->load(path))
        {
            Fatal("Couldn't load the image '%.*s'", (i32)path.len, path.items);
        }
    }
    
    Texture2D* tex = nullptr;
    if(data.cached_images.has(image))
    {
        tex = (Texture2D*)data.cached_images.get(image);
    }
    else
    {
        Debug::info("Loading the texture '%.*s'...", (i32)path.len, path.items);
        tex = get_allocator().object<Texture2D>();
        tex->type = RESOURCE_TEXTURE_2D;
        tex->path = String::from_chars(get_allocator(), path);
        
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
        Debug::info("'%.*s' was loaded correctly.", (i32)path.len, path.items);
    }
    
    return tex;
}


SpriteAnimation* ResourceManager::create_sprite_animation(StringView name)
{
    if (data.resources.has(name))
    {
        return nullptr;
    }

    SpriteAnimation* sprite_animation = get_allocator().object<SpriteAnimation>();
    data.resources.insert(name, sprite_animation);

    sprite_animation->type = RESOURCE_SPRITE_ANIMATION;
    sprite_animation->path = String::from_chars(get_allocator(), name);
    sprite_animation->animations = StringMap<SpriteAnimation::Animation>::with_allocator(get_allocator());
    return sprite_animation;
}
