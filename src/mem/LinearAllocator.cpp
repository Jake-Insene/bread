#include "Mem/LinearAllocator.hpp"


namespace Mem
{

LinearAllocator::LinearAllocator(const Slice<u8>& new_sp)
{
    sp = new_sp;
    offset = 0;
}

void LinearAllocator::reset()
{
    offset = 0;
}

Slice<u8> LinearAllocator::alloc(usize size, usize alignment)
{
    const usize aligned_size = Mem::align_up(size, alignment);
    const usize aligned_sp = Mem::align_up(offset, alignment);
    const usize aligned_offset = aligned_sp - offset;

    if(offset + aligned_size + aligned_offset >= sp.len)
    {
        return {};
    }

    Slice<u8> ptr = {};
    ptr = sp.add(offset + aligned_offset);
    ptr.len = size;

    offset += aligned_offset + aligned_size;

    return ptr;
}

bool LinearAllocator::realloc([[maybe_unused]] const Slice<u8>& ptr, [[maybe_unused]] usize new_size,
    [[maybe_unused]] usize alignment)
{
    return false;
}

Slice<u8> LinearAllocator::remap([[maybe_unused]] const Slice<u8>& ptr, [[maybe_unused]] usize new_size,
    [[maybe_unused]] usize alignment)
{
    return {};
}

void LinearAllocator::free([[maybe_unused]] const Slice<u8>& ptr)
{}

}
