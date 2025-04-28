#include "io/resource_manager.h"

#include "debug/debug.h"
#include "graphics/graphics.h"

#include <external/stb_image.h>


void ResourceManager::initialize(mem::Allocator& allocator)
{
    data = {};
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
    for(auto& entry : data.resources)
    {
        if(entry.value->type == RESOURCE_IMAGE)
        {
            Image* image = (Image*)entry.value;
            image->destroy();
        }
        
        data.allocator.free(
            mem::to_bytes(Slice<Image>((Image*)entry.value, 1))
        );
    }
    
    for(auto& entry : data.cached_images)
    {
        if(entry.value)
        {
            entry.value->destroy();
            data.allocator.free(
                mem::to_bytes(Slice<Texture>(entry.value, 1))
            );
        }
    }

    data.resources.destroy();
    data.cached_images.destroy();
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
        image = data.allocator.object<Image>();

        image->type = RESOURCE_IMAGE;
        image->path = String::from_chars(data.allocator, path);
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
        tex = data.allocator.object<Texture2D>();
        tex->type = RESOURCE_TEXTURE_2D;
        tex->path = String::from_chars(data.allocator, path);
        
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

