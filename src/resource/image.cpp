#include "resource/image.h"

#include "io/file.h"
#include "resource/resource_manager_internal.h"

#include <external/stb_image.h>


void Image::destroy()
{
    if(!pixels.null())
    {
        allocator.free(pixels);
    }

    Resource::destroy();
}

Error Image::load(StringView file_path)
{
    if (File::exists(file_path) == false)
    {
        RMDebugInfo("Couldn't load the font '{}'", file_path);
        return MakeError(ErrorCode::FileNotFound);
    }

    Slice<u8> buffer = File::read_all(allocator, file_path);
    
    i32 channels = 0;
    pixels.items = reinterpret_cast<u8*>(stbi_load_from_memory(
        buffer.ptr(), static_cast<int>(buffer.len), &size.width, &size.height, &channels, 0
    ));

    if(pixels.null())
    {
        return MakeError(ErrorCode::ImageCorrupted);
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
        RMFatal("invalid channel count {}", channels);
    }
    
    pixels.len = size.width * size.height * channels;
    allocator.free(buffer);
    
    return ErrorCode::Ok;
}

void Image::unload()
{
    if (!pixels.null())
    {
        allocator.free(pixels);
        pixels = Slice<u8>(nullptr, 0);
        size = Vector2I();
        format = Image::FORMAT_UNKNOWN;
    }
}
