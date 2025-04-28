#include "mem/page_allocator.h"

#include "mem/utils.h"
#include "platform/platform_header.h"

namespace mem
{

    Slice<u8> PageAllocator::alloc(usize size, usize alignment)
    {
        Slice<u8> ptr = {};

        const usize aligned_size = mem::align_up(size, alignment);

        ptr.items = (u8*)mmap(
                0, aligned_size,
                PROT_READ | PROT_WRITE,
                MAP_ANONYMOUS | MAP_PRIVATE,
                -1, 0
        );
        ptr.len = aligned_size;

        return ptr;
    }

    bool PageAllocator::realloc(Slice<u8> ptr, usize new_size, usize)
    {
        const usize aligned_new_size = mem::align_up(new_size, mem::get_page_size());

        const usize aligned_ptr_size = mem::align_up(ptr.len, mem::get_page_size());
        if (aligned_new_size == aligned_ptr_size)
            return true;

        if (aligned_new_size < aligned_ptr_size) {
            u8* ptr_out = ((u8*)ptr.items) + aligned_new_size;
            munmap(ptr_out, aligned_ptr_size - aligned_new_size);
            return true;
        }

        return false;
    }

    void PageAllocator::free(Slice<u8> ptr)
    {
        munmap(ptr.items, ptr.len);
    }

    Allocator PageAllocator::allocator()
    {
        return Allocator
        {
            .vtable =
            {
                .alloc = (decltype(Allocator::VTable::alloc))&PageAllocator::alloc,
                .realloc = (decltype(Allocator::VTable::realloc))&PageAllocator::realloc,
                .free = (decltype(Allocator::VTable::free))&PageAllocator::free,
            },
            .self = (Allocator*)this,
        };
    }
}
