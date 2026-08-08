#pragma once
#include "Core/Types.h"


namespace Core
{

/*
* Use it to operate with memory of unknown type.
* You can't construct a Opaque type, instead you need reinterpret the
* memory address as an Opaque.
*/
struct Opaque
{
    Opaque() = delete;
    Opaque(const Opaque&) = delete;
    Opaque(Opaque&&) = delete;

    template<typename T>
    static Opaque* from(T& reference) { return reinterpret_cast<Opaque*>(&reference); }

    /*
    * Returns the address of the object as a integral number.
    */
    [[nodiscard]] MemoryAddress address() const { return reinterpret_cast<MemoryAddress>(this); }

    /**
    * Reinterprets the memory as a diferent type.
    * @tparam Type the type of the object to cast to.
    */
    template<typename Type, typename Self>
    [[nodiscard]] Type cast(this Self& self)
    {
        return reinterpret_cast<Type>(&self);
    };
};

}
