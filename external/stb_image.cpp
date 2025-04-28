#include "io/resource_manager.h"

#include <memory>

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
    
    void* new_mem = ResourceManager::get_allocator().alloc(new_size, alignof(usize)).items;
    if(ptr)
    {
        std::memcpy(new_mem, ptr, old_size);
    }
    
    return new_mem;
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
#include <external/stb_image.h>
