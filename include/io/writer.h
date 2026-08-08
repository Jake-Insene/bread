#pragma once
#include "Core/Header.h"


template<typename T>
struct Slice;

namespace IO
{

/*
* Represents a writable memory object/segment.
*/
struct Writer
{
    Core::Opaque* writable;
    void(*write_fn)(Core::Opaque*, const Slice<const u8>&);

    /*
    * Writes the given bytes to the object/segment.
    *
    * @param bytes The bytes to write.
    */
    void write(const Slice<const u8>& bytes) const;
};

template<typename T>
concept Writable = requires(T&& object, const Slice<const u8>&& bytes)
{
    { object.writer() } -> Core::ConvertibleTo<Writer>;
    { object.writer().write(bytes) };
};

}