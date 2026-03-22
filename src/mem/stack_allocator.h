#pragma once
#include "mem/allocator.h"


namespace mem
{

struct StackAllocator
{
    Slice<u8> sp;
    usize offset;

    void init(Slice<u8> new_sp);
    void reset();

    Slice<u8> alloc(usize size, usize alignment);
    bool realloc(Slice<u8> ptr, usize new_size, usize alignment);
    void free(Slice<u8> ptr);
    usize get_size_of(Slice<u8> ptr) const;
    
    Allocator allocator();
};

}