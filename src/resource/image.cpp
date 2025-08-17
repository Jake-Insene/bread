#include "resource/image.h"

#include "fs/file.h"
#include "resource/resource_manager.h"

#include <external/stb_image.h>


void Image::init()
{
    Resource::init(RESOURCE_IMAGE);
}

void Image::destroy()
{
    Resource::destroy();
    if(!pixels.null())
    {
        ResourceManager::get_allocator().free(pixels);
    }
}

bool Image::load(StringView file_path)
{
    if (File::exists(file_path) == false)
    {
        Fatal("Couldn't load the image '{}'", file_path);
        return false;
    }

    Slice<u8> buffer = File::read_all(ResourceManager::get_allocator(), file_path);
    
    i32 channels = 0;
    pixels.items = (u8*)stbi_load_from_memory(
        buffer.ptr(), (int)buffer.len, &size.width, &size.height, &channels, 0
    );
    
    if(pixels.null())
    {
        return false;
    }
    
    if(channels == 3)
    {
        format = FORMAT_RGB8;
    }
    else if(channels == 4)
    {
        format = FORMAT_RGBA8;
    }
    else
    {
        Fatal("invalid channel count {}", channels);
    }
    
    pixels.len = size.width * size.height * channels;
    ResourceManager::get_allocator().free(buffer);
    
    return true;
}

void Image::unload()
{
    if (!pixels.null())
    {
        ResourceManager::get_allocator().free(pixels);
        pixels = {};
        size = {};
        format = Image::FORMAT_UNKNOWN;
    }
}
