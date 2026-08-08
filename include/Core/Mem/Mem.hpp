#pragma once
#include "Core/Templates.h"

// Required in Clang.
#include <new>


namespace Core::Mem
{

template<typename T>
constexpr auto AddressOf(T& reference)
{
    if constexpr(IsPointer<T>)
    {
        return reference;
    }
    else
    {
        return &reference;
    }
}

template<typename T, typename... TArgs>
constexpr void Placement(T& object, TArgs&&... args)
{
    ::new(&object) T(Core::Forward<TArgs>(args)...);
}

template<typename T, typename... TArgs>
constexpr void PlacementArray(T* array, usize len, TArgs&&... args)
{
    ::new(array) T[len](Core::Forward(args)...);
}

template<typename T>
requires(std::is_object_v<T>)
constexpr void Destruct(T& object)
{
    object.~T();
}

}