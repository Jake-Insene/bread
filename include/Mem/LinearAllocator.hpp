#pragma once
#include "Mem/Allocator.hpp"


namespace Mem
{

struct LinearAllocator : Mem::Allocator
{
    Slice<u8> sp;
    usize offset;

    LinearAllocator(const Slice<u8>& new_sp);
    
    void reset();

    virtual Slice<u8> alloc(usize size, usize alignment) override;
    virtual bool realloc(const Slice<u8>& ptr, usize new_size, usize alignment) override;
    virtual Slice<u8> remap(const Slice<u8>& ptr, usize new_size, usize alignment) override;
    virtual void free(const Slice<u8>& ptr) override;
};

}