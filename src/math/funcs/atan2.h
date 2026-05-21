#pragma once
#include "math/constants.h"
#include "math/funcs/abs.h"


namespace Math::impl
{

// TODO:
// I don't know how this works.
template<typename T>
    requires(IsArithmetic<T>)
constexpr T atan2_approx(T y, T x)
{
    if (x == T(0.0))
    {
        if (y > T(0.0))
            return PI_HALF<T>;
        if (y < T(0.0))
            return -PI_HALF<T>;
        return 0.0f;
    }

    T atan;
    T z = y / x;

    if (Math::abs(z) < T(1.0))
    {
        atan = z / (T(1.0) + T(0.28) * z * z);
        if (x < T(0.0))
        {
            if (y < T(0.0))
                return atan - PI<T>;
            else
                return atan + PI<T>;
        }
    }
    else
    {
        atan = PI_HALF<T> - z / (z * z + T(0.28));
        if (y < T(0.0))
            return atan - PI<T>;
    }

    return atan;
}

}