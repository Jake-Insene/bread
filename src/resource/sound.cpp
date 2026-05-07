#include "resource/sound.h"

#include "resource/resource_manager.h"
#include "resource/resource_manager_internal.h"
#include "engine/engine.h"


static inline void* _dr_alloc(size_t size, void*)
{
    mem::Allocator allocator = Engine::get_resource_manager()->get_allocator();
    return allocator.alloc(size, 16).items;
}

static inline void* _dr_realloc(void* mem, size_t new_size, void*)
{
    Slice<u8> old_mem = Slice(reinterpret_cast<u8*>(mem), 1);
    mem::Allocator allocator = Engine::get_resource_manager()->get_allocator();
    if (allocator.realloc(old_mem, new_size, 16))
    {
        return mem;
    }

    Slice<u8> new_mem = allocator.alloc(new_size, 16);
    if (mem != nullptr)
    {
        allocator.free(old_mem);
    }

    return new_mem.items;
}

static inline void _dr_free(void* mem, void*)
{
    mem::Allocator allocator = Engine::get_resource_manager()->get_allocator();
    allocator.free(
        Slice(reinterpret_cast<u8*>(mem), 1)
    );
}

static inline drwav_allocation_callbacks alloc_callbacks =
{
    .pUserData = nullptr,
    .onMalloc = &_dr_alloc,
    .onRealloc = &_dr_realloc,
    .onFree = &_dr_free,
};

void Sound::init(const ResourceCreateInfo& info)
{
    Resource::init(info);

    data.buffer = {};
}

void Sound::destroy()
{
    if(!data.buffer.null())
    {
        allocator.free(data.buffer);
    }

    Resource::destroy();
}

Error Sound::load(StringView file_path)
{
    if (!File::exists(file_path))
    {
        RMDebugInfo("Couldn't load the font '{}'", file_path);
        return MakeError(ErrorCode::FileNotFound);
    }
    
    path.set(file_path);

    Slice<u8> content = File::read_all(allocator, file_path);

    drwav wav = {};
    drwav_init_memory(&wav, content.ptr(), content.len, &alloc_callbacks);

    if(wav.channels > Audio::output_get_channels())
    {
        RMDebugInfo("The WAV file({}) contains more channels than are supported, find({}), supported({})",
            file_path, wav.channels, Audio::output_get_channels()
        );
    }

    const size_t total_samples = static_cast<size_t>(wav.totalPCMFrameCount * wav.channels);
    const usize bytes_per_sample = wav.bitsPerSample / 8;
    data.buffer = allocator.array<u8>(total_samples * bytes_per_sample);

    (void)drwav_read_pcm_frames_f32(&wav, wav.totalPCMFrameCount, reinterpret_cast<f32*>(data.buffer.ptr()));

    drwav_uninit(&wav);
    allocator.free(content);

    return ErrorCode::Ok;
}
