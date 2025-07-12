#pragma once
#include "core/macros.h"
#include "core/templates.h"

namespace math::impl
{

// I don't know how this works.
template<typename T>
T sin_approx(T r)
{
    static_assert(
        IsArithmetic<T>,
        "expected arithmetic type"
    );
    // Simple Taylor approximation for small angles
    T r2 = r * r;
    return r - (r2 * r) / T(6.0) + (r2 * r2 * r) / T(120.0);
}

}
