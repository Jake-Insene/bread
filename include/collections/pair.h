#pragma once
#include "core/templates.h"



template<typename T1, typename T2>
struct [[nodiscard]] Pair
{
    T1 first;
    T2 second;

    Pair() {}

    Pair(T1 _first, T2 _second)
    : first(_first), second(_second)
    {}
};
