#include "resource/sound.h"

#include "resource/resource_manager.h"


static void* _dr_alloc(size_t size, void*)
{
    return ResourceManager::get_allocator().alloc(size, 16).items;
}

static void* _dr_realloc(void* mem, size_t new_size, void*)
{
    Slice<u8> old_mem = Slice((u8*)mem, 1);
    if (ResourceManager::get_allocator().realloc(old_mem, new_size, 16))
    {
        return mem;
    }

    Slice<u8> new_mem = ResourceManager::get_allocator().alloc(new_size, 16);
    if (mem != nullptr)
    {
        ResourceManager::get_allocator().free(old_mem);
    }

    return new_mem.items;
}

static inline void _dr_free(void* mem, void*)
{
    ResourceManager::get_allocator().free(Slice((u8*)mem, 1));
}

static inline drwav_allocation_callbacks alloc_callbacks =
{
    .pUserData = nullptr,
    .onMalloc = &_dr_alloc,
    .onRealloc = &_dr_realloc,
    .onFree = &_dr_free,
};


void Sound::init()
{
	Resource::init(RESOURCE_SOUND);
}

void Sound::destroy()
{
	Resource::destroy();
    Audio::destroy_source_voice(data.source_voice);
}

void Sound::load(StringView file_path)
{
    if (File::exists(file_path) == false)
    {
        Fatal("Couldn't load the sound file '{}'", file_path);
        return;
    }

    auto allocator = ResourceManager::get_allocator();
    path.set(file_path);

    Slice<u8> content = File::read_all(allocator, file_path);

    drwav wav;
    drwav_init_memory(&wav, content.ptr(), content.len, &alloc_callbacks);

    size_t total_samples = (size_t)wav.totalPCMFrameCount * wav.channels;
    auto buffer = allocator.array<u8>(total_samples * sizeof(i16));
    drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, (i16*)buffer.ptr());

    AudioSourceVoiceCreateInfo sv_create_info;

    // TODO: This is only for 16-bits
    sv_create_info.awf.channel_number = wav.channels;
    sv_create_info.awf.samples_per_sec = wav.sampleRate;
    sv_create_info.awf.bits_per_sample = 16;
    sv_create_info.awf.block_align = wav.channels * 2;
    sv_create_info.awf.avg_bytes_per_sec = wav.sampleRate * sv_create_info.awf.block_align;
    sv_create_info.buffer = buffer;

    data.source_voice = Audio::create_source_voice(sv_create_info);

    drwav_uninit(&wav);
    allocator.free(buffer);
    allocator.free(content);
}
