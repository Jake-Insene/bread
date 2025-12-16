#pragma once
#include "core/types.h"

using MemoryAddress = usize;


/*
* Use it to operate with a object of unknown type.
* You can't construct a Opaque type, instead you need reinterpret the
* object pointer as an Opaque type.
*/
struct Opaque
{
    Opaque() = delete;
    Opaque(const Opaque&) = delete;
    Opaque(Opaque&&) = delete;

    [[nodiscard]] MemoryAddress address() const { return reinterpret_cast<MemoryAddress>(this); }

    template<typename Type>
    [[nodiscard]] Type cast() const { return reinterpret_cast<Type>(address()); };
};