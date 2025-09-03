#pragma once
#include "math/funcs/floor.h"

namespace math
{

template<typename T>
[[nodiscard]] constexpr T log2(T n)
{
    static_assert(IsArithmetic<T>, "expected arithmetic type");

    if constexpr (IsInteger<T>)
    {
        T result = 0;
        while (n >>= 1)
        {
            result++;
        }
        return result;
    }
    else
    {
        // type is floating point;
        T result = 0;
        i64 n_convert = i64(math::floor(n));
        while (n_convert >>= 1)
        {
            result++;
        }
        return result;
    }
}

}