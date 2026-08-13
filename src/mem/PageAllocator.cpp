#include "Mem/PageAllocator.hpp"

#include "debug/fail.h"
#include "Mem/Utils.hpp"
#include "os/os.h"


namespace Mem
{

Slice<u8> PageAllocator::alloc(usize size, usize)
{
    Slice<u8> ptr = {};

    const usize aligned_size = Mem::align_up(size, OS::get_page_size());

    ptr = OS::map_memory(aligned_size, OS::ReadWrite);
    ptr.len = aligned_size;

#if defined(DEBUG)
    Mem::set(ptr, u8(0xCD));
#endif

    return ptr;
}

bool PageAllocator::realloc(const Slice<u8>& ptr, usize new_size, usize)
{
    // TODO: check this code later, may not work correctly
    const usize aligned_new_size = Mem::align_up(new_size, OS::get_page_size());

    const usize aligned_ptr_size = Mem::align_up(ptr.len, OS::get_page_size());
    if (aligned_new_size == aligned_ptr_size)
    {
        return true;
    }

    if (aligned_new_size < aligned_ptr_size)
    {
        u8* ptr_out = ptr.ptr() + aligned_new_size;
        Slice memory_to_free = Slice(ptr_out, aligned_ptr_size - aligned_new_size);
        OS::unmap_memory(memory_to_free);
        return true;
    }

    return false;
}

Slice<u8> PageAllocator::remap([[maybe_unused]] const Slice<u8>& ptr, [[maybe_unused]] usize new_size,
    [[maybe_unused]] usize alignment)
{
    FailOn(true, "PageAllocator::remap is not implemented yet");
    return {};
}

void PageAllocator::free(const Slice<u8>& ptr)
{
    OS::unmap_memory(ptr);
}

}
