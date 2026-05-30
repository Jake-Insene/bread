#include "resource/image.h"

#include "io/file.h"
#include "resource/resource_manager_internal.h"

#include <external/stb_image.h>


void Image::init(const ResourceCreateInfo& info)
{
    Resource::init(info);

    data.pixels = {};
    data.size = {};
    data.format = {};
}

void Image::destroy()
{
    if(!data.pixels.null())
    {
        allocator->free(data.pixels);
    }

    Resource::destroy();
}

Error Image::load(StringView file_path)
{
    if (!IO::File::exists(allocator, file_path))
    {
        RMDebugInfo("Couldn't load the font '{}'", file_path);
        return MakeError(ErrorCode::FileNotFound);
    }

    Slice<u8> buffer = IO::File::read_all(allocator, file_path);
    
    i32 channels = 0;
    data.pixels.items = reinterpret_cast<u8*>(stbi_load_from_memory(
        buffer.ptr(), static_cast<int>(buffer.len), &data.size.width, &data.size.height, &channels, 0
    ));

    if(data.pixels.null())
    {
        return MakeError(ErrorCode::ImageCorrupted);
    }
    
    if(channels == 3)
    {
        data.format = ImageFormat::RGB8;
    }
    else if(channels == 4)
    {
        data.format = ImageFormat::RGBA8;
    }
    else
    {
        RMFatal("invalid channel count {}", channels);
    }
    
    data.pixels.len = isize(data.size.width * data.size.height * channels);
    allocator->free(buffer);
    
    return ErrorCode::Ok;
}

void Image::unload()
{
    if (!data.pixels.null())
    {
        allocator->free(data.pixels);
        data.pixels = {};
        data.size = {};
        data.format = {};
    }
}
