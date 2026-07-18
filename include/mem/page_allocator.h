#pragma once
#include "mem/allocator.h"


namespace Mem
{
    
struct PageAllocator : Mem::Allocator
{
    virtual Slice<u8> alloc(usize size, usize alignment) override;
    virtual bool realloc(const Slice<u8>& ptr, usize new_size, usize alignment) override;
    virtual void free(const Slice<u8>& ptr) override;
    virtual usize get_size_of(const Slice<u8>& ptr) const override;
};
    
}

