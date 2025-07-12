#pragma once
#include "core/macros.h"
#include "core/templates.h"

namespace math::impl
{

// I don't know how this works.
template<typename T>
T cos_approx(T r)
{
    static_assert(
        IsArithmetic<T>,
        "expected arithmetic type"
    );
    T r2 = r * r;
    return 1 - (r2)/2;
}

}