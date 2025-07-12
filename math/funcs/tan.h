#pragma once
#include "math/funcs/sin.h"

namespace math::impl
{

// I don't know how this works.
template<typename T>
T tan_approx(T r)
{
    static_assert(
        IsArithmetic<T>,
        "expected arithmetic type"
    );
    return sin_approx(r);
}

}