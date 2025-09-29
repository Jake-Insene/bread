#include "resource/resource_manager.h"


static void* _alloc(usize size)
{
    return ResourceManager::get_allocator().alloc(size, alignof(usize)).items;
}

static void* _realloc(void* ptr, usize old_size, usize new_size)
{
    Slice<u8> old_mem = Slice<u8>{(u8*)ptr, old_size};
    if(ptr && ResourceManager::get_allocator().realloc(old_mem, new_size, alignof(usize)))
    {
        return ptr;
    }
    
    Slice<u8> new_mem = ResourceManager::get_allocator().alloc(new_size, alignof(usize));
    if (new_mem.ptr() && ptr != nullptr && old_size != 0)
    {
        mem::copy(new_mem, old_mem);
        ResourceManager::get_allocator().free(old_mem);
    }
    
    return new_mem.ptr();
}

static void _free(void* ptr)
{
    if(ptr)
    {
        ResourceManager::get_allocator().free(Slice<u8>((u8*)ptr, 1));
    }
}

static void* __bread_memcpy(void* dest, void* src, size_t len)
{
    auto dest_items = Slice((u8*)dest, len);
    auto src_items = Slice((u8*)src, len);
    mem::copy(dest_items, src_items);
    return dest;
}

static void* __bread_memset(void* dest, int value, size_t len)
{
    auto dest_items = Slice((u8*)dest, len);
    mem::set(dest_items, u8(value));
    return dest;
}

static u32 _lrotl(u32 x, int y)
{
    return (((x) << (y)) | ((x) >> (-(y) & 31)));
}

#define stbi_err(x) DebugAssert("{}", x);

#define STBI_MALLOC(size) _alloc(size)
#define STBI_REALLOC(ptr, new_size) _realloc(ptr, 0, new_size)
#define STBI_FREE(ptr) _free(ptr)

#define STBI_REALLOC_SIZED(ptr, old_size, new_size)  _realloc(ptr, old_size, new_size)

#define STBTT_malloc(size, u) _alloc(size)
#define STBTT_free(ptr, u) _free(ptr)

#define STBTT_assert(x)
#define STBTT_strlen(x) __string_len(x)

#define STB_IMAGE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION

#define STBI_NO_THREAD_LOCALS
#define STBI_NO_STDIO
#define STBI_NO_LINEAR
#define STBI_NO_HDR
#define STBI_ASSERT(x) DebugAssert(x, "stb_image assertion fail")

#define abs(x) math::abs(x)
#define fabs(x) abs(x)

#include "external/stb_image.h"

#undef abs
#undef fabs

#define STBTT_ifloor(x) i32(math::floor<f32>(x))
#define STBTT_iceil(x) i32(math::ceil<f32>(x))
#define STBTT_sqrt(x) math::sqrt(x)
#define STBTT_pow(x, y) math::pow(x, y)
#define STBTT_fmod(x, y) math::mod(x, y)
#define STBTT_cos(x) math::cos(x)
#define STBTT_acos(x) math::acos(x)
#define STBTT_fabs(x) math::abs(x)

#define STBTT_memcpy(dest, src, size) __bread_memcpy(dest, src, size)
#define STBTT_memset(dest, value, size) __bread_memset(dest, value, size)

#include "external/stb_truetype.h"

#define DRWAV_ASSERT(...)
#define DRWAV_MALLOC(size) _alloc(size)
#define DRWAV_REALLOC(ptr, size) _realloc(ptr, 0, size)
#define DRWAV_FREE(ptr) _free(ptr)
#define DRWAV_COPY_MEMORY(dest, src, len) __bread_memcpy((void*)(dest), (void*)(src), len)
#define DRWAV_ZERO_MEMORY(dest, len) __bread_memset((void*)(dest), 0, len)

#define DR_WAV_NO_STDIO
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
