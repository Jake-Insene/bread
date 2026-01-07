#pragma once
#include "core/header.h"


template<typename T>
struct Slice;

namespace io
{

/*
* Represents a writable memory object/segment.
*/
struct Writer
{
    Opaque* writable;
    void(*write_fn)(Opaque*, const Slice<const u8>&);

    /*
    * Writes the given bytes to the object/segment.
    * 
    * @param bytes The bytes to write.
    */
    void write(const Slice<const u8>& bytes) const;
};

}