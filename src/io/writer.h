#pragma once
#include "core/types_ext.h"

template<typename T>
struct Slice;

namespace io
{
    
struct Writer : Opaque
{
    void(*write_fn)(void*, const Slice<const u8>);

    void write(const Slice<const u8> bytes) const;
};

}