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
    if(new_mem.ptr() && ptr != nullptr && old_size != 0)
    {
        mem::copy(old_mem, new_mem);
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

#define STBI_MALLOC(size) _alloc(size)
#define STBI_REALLOC(ptr, new_size) _realloc(ptr, 0, new_size)
#define STBI_FREE(ptr) _free(ptr)

#define STBI_REALLOC_SIZED(ptr, old_size, new_size)  _realloc(ptr, old_size, new_size)

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_LINEAR
#define STBI_NO_HDR
#define STBI_ASSERT(x) DebugAssert(x, "stb_image assertion fail")

#include <memory>
#define _lrotl(x, y) (((x) << (y)) | ((x) >> (-(y) & 31)))

#include "external/stb_image.h"
