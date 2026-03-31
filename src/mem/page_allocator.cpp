#include "mem/page_allocator.h"

#include "mem/utils.h"
#include "os/os.h"


namespace mem
{

Slice<u8> PageAllocator::alloc(usize size, usize)
{
    Slice<u8> ptr = {};

    const usize aligned_size = mem::align_up(size, OS::get_page_size());

    ptr = OS::map_memory(aligned_size, OS::MapReadWrite);
    ptr.len = aligned_size;

    return ptr;
}

bool PageAllocator::realloc(Slice<u8> ptr, usize new_size, usize)
{
    const usize aligned_new_size = mem::align_up(new_size, OS::get_page_size());

    const usize aligned_ptr_size = mem::align_up(ptr.len, OS::get_page_size());
    if (aligned_new_size == aligned_ptr_size)
        return true;

    if (aligned_new_size < aligned_ptr_size)
    {
        u8* ptr_out = ptr.ptr() + aligned_new_size;
        Slice<u8> memory_to_free = Slice<u8>(ptr_out, aligned_ptr_size - aligned_new_size);
        OS::unmap_memory(memory_to_free);
        return true;
    }

    return false;
}

void PageAllocator::free(Slice<u8> ptr)
{
    OS::unmap_memory(ptr);
}

usize PageAllocator::get_size_of(Slice<u8> ptr) const
{
    return OS::query_memory(ptr).region_size;
}

static inline Allocator::VTable page_vtable =
{
    .alloc = reinterpret_cast<decltype(Allocator::VTable::alloc)>(&PageAllocator::alloc),
    .realloc = reinterpret_cast<decltype(Allocator::VTable::realloc)>(&PageAllocator::realloc),
    .free = reinterpret_cast<decltype(Allocator::VTable::free)>(&PageAllocator::free),
    .get_size_of = reinterpret_cast<decltype(Allocator::VTable::get_size_of)>(&PageAllocator::get_size_of),
};

Allocator PageAllocator::allocator()
{
    return Allocator
    {
        .vtable = &page_vtable,
        .self = reinterpret_cast<Allocator*>(this),
    };
}
}
