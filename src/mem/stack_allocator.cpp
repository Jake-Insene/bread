#include "mem/stack_allocator.h"


namespace mem
{

void StackAllocator::init(Slice<u8> new_sp)
{
    sp = new_sp;
    offset = 0;
}

void StackAllocator::reset()
{
    offset = 0;
}

Slice<u8> StackAllocator::alloc(usize size, usize alignment)
{
    const usize aligned_size = mem::align_up(size, alignment);

    if(offset + aligned_size == sp.len)
    {
        return Slice<u8>();
    }

    Slice<u8> ptr = {};
    ptr = sp.add(offset);
    ptr.len = size;

    offset += aligned_size;

    return ptr;
}

bool StackAllocator::realloc(Slice<u8> ptr, usize new_size, usize alignment)
{
    Unused(ptr, new_size, alignment);
    return false;
}

void StackAllocator::free(Slice<u8> ptr)
{
    Unused(ptr);
}

usize StackAllocator::get_size_of(Slice<u8> ptr) const
{
    Unused(ptr);
    return MaxValue<usize>;
}

static inline Allocator::VTable sa_vtable = 
{
    .alloc = reinterpret_cast<decltype(Allocator::VTable::alloc)>(&StackAllocator::alloc),
    .realloc = reinterpret_cast<decltype(Allocator::VTable::realloc)>(&StackAllocator::realloc),
    .free = reinterpret_cast<decltype(Allocator::VTable::free)>(&StackAllocator::free),
    .get_size_of = reinterpret_cast<decltype(Allocator::VTable::get_size_of)>(&StackAllocator::get_size_of),
};
Allocator StackAllocator::allocator()
{
    return Allocator
    {
        .vtable = &sa_vtable,
        .self = reinterpret_cast<Allocator*>(this),
    };
}

}
