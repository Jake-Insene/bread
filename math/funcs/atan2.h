#pragma once
#include "math/constants.h"
#include "math/funcs/abs.h"


namespace math::impl
{

// I don't know how this works.
template<typename T>
constexpr T atan2_approx(T x, T y)
{
    static_assert(
        IsArithmetic<T>,
        "expected arithmetic type"
    );

    if (x == T(0.0))
    {
        if (y > T(0.0)) return PI2<T>;
        if (y < T(0.0)) return -PI2<T>;
        return 0.0f;
    }

    T atan;
    T z = y / x;

    if (math::abs(z) < T(1.0))
    {
        atan = z / (T(1.0) + T(0.28) * z * z);
        if (x < T(0.0))
        {
            if (y < T(0.0)) return atan - PI<T>;
            else          return atan + PI<T>;
        }
    }
    else
    {
        atan = PI2<T> - z / (z * z + T(0.28));
        if (y < T(0.0)) return atan - PI<T>;
    }

    return atan;
}

}