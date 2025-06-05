#pragma once
#include "collections/slice.h"
#include "core/types_ext.h"

namespace io
{
    
struct Writer : Opaque
{
    void(*write_fn)(void*, const Slice<const u8>);

    void write(const Slice<const u8> bytes) const
    {
        write_fn(self, bytes);
    }
};

}