#pragma once
#include "core/doc.h"
#include "core/types.h"



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

    
    template<typename T>
    static Opaque* from(T& reference) { return reinterpret_cast<Opaque*>(&reference); }

    /*
    * Returns the address of the object as a integral number.
    */
    [[nodiscard]] MemoryAddress address() const { return reinterpret_cast<MemoryAddress>(this); }

    /*
    * Reinterprets the object as a diferent type.
    * @tparam Type the type of the object to cast to.
    */
    template<typename Type, typename Self>
    [[nodiscard]] Type cast(this Self& self) Function(FunctionUnsafe)
    {
        return reinterpret_cast<Type>(&self);
    };
};