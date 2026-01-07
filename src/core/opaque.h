#pragma once
#include "core/doc.h"
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

    /*
    * Returns the address of the object as a plain number.
    */
    [[nodiscard]] MemoryAddress address() const { return reinterpret_cast<MemoryAddress>(this); }

    /*
    * Reinterprets the object as a diferent type.
    * @tparam Type the type of the object to cast to.
    */
    template<typename Type>
    [[nodiscard]] inline Type cast() const Function(FunctionUnsafe)
    {
        return reinterpret_cast<Type>(address());
    };
};