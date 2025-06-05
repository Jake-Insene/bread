#pragma once
#include "core/templates.h"

namespace math
{

template<typename T>
constexpr T abs(T a)
{
    static_assert(IsArithmetic<T>, "expected arithmetic type");
    return a >= 0 ? a : -a;
}

}
