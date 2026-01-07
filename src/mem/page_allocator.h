#pragma once
#include "mem/allocator.h"

namespace mem
{
    
    struct PageAllocator
    {
        Slice<u8> alloc(usize size, usize alignment);
        bool realloc(Slice<u8> ptr, usize new_size, usize alignment);
        void free(Slice<u8> ptr);
        
        Allocator allocator();
    };
    
}

