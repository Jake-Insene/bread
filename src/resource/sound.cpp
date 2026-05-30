#include "resource/sound.h"

#include "resource/resource_manager.h"
#include "resource/resource_manager_internal.h"
#include "engine/engine.h"


static inline void* dr_alloc(size_t size, void* user_data)
{
    Unused(user_data);
    Mem::Allocator* allocator = Engine::get_resource_manager()->get_allocator();
    return allocator->alloc(size, 16).items;
}

static inline void* dr_realloc(void* mem, size_t new_size, void* user_data)
{
    Unused(user_data);
 
    Slice<u8> old_mem = Slice(reinterpret_cast<u8*>(mem), 1);
    Mem::Allocator* allocator = Engine::get_resource_manager()->get_allocator();
    if (allocator->realloc(old_mem, new_size, 16))
    {
        return mem;
    }

    Slice<u8> new_mem = allocator->alloc(new_size, 16);
    if (mem != nullptr)
    {
        allocator->free(old_mem);
    }

    return new_mem.items;
}

static inline void dr_free(void* mem, void* user_data)
{
    Unused(user_data);

    Mem::Allocator* allocator = Engine::get_resource_manager()->get_allocator();
    allocator->free(
        Slice(reinterpret_cast<u8*>(mem), 1)
    );
}

static inline drwav_allocation_callbacks alloc_callbacks =
{
    .pUserData = nullptr,
    .onMalloc = &dr_alloc,
    .onRealloc = &dr_realloc,
    .onFree = &dr_free,
};

void Sound::init(const ResourceCreateInfo& info)
{
    Resource::init(info);

    data.mono = false;
    data.samples = {};
}

void Sound::destroy()
{
    if(!data.samples.null())
    {
        allocator->free(Mem::to_bytes(data.samples));
    }

    Resource::destroy();
}

Error Sound::load(StringView file_path)
{
    if (!IO::File::exists(allocator, file_path))
    {
        RMDebugInfo("Couldn't load the font '{}'", file_path);
        return MakeError(ErrorCode::FileNotFound);
    }
    
    path.set(file_path);

    Slice<u8> content = IO::File::read_all(allocator, file_path);

    drwav wav = {};
    drwav_init_memory(&wav, content.ptr(), content.len, &alloc_callbacks);

    if(wav.channels != 1 && wav.channels != 2)
    {
        RMDebugInfo("The WAV file({}) contains a not supported channel count, find({}) expected 1 or 2",
            file_path, wav.channels
        );
    }

    const usize total_samples = static_cast<usize>(wav.totalPCMFrameCount * wav.channels);
    data.mono = wav.channels == 1;
    data.samples = allocator->array<i16>(total_samples);

    // Always convert 
    (void)drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, reinterpret_cast<drwav_int16*>(data.samples.ptr()));

    drwav_uninit(&wav);
    allocator->free(content);

    return ErrorCode::Ok;
}

Audio::Frame Sound::get_frame(usize index) const
{
    if(is_stereo())
    {
        return Audio::Frame(
            data.samples[(index * Audio::OutputChannels) + 0],
            data.samples[(index * Audio::OutputChannels) + 1]
        );
    }

    return Audio::Frame(
        data.samples[index],
        data.samples[index]
    );
}
