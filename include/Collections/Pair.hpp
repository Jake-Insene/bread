#pragma once
#include "Core/Header.hpp"
#include "Collections/Tuple.hpp"



template<typename T1, typename T2>
struct [[nodiscard]] Pair
{
    T1 first;
    T2 second;

    Pair() {}

    Pair(T1 _first, T2 _second)
    : first(_first), second(_second)
    {}

    template<typename... TArgs1, typename... TArgs2>
    Pair(Tuple<TArgs1...> args1, Tuple<TArgs2...> args2)
    : first(FromTuple<T1>(args1)), second(FromTuple<T2>(args2))
    {}
};
