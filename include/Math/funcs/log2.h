#pragma once
#include "Math/funcs/floor.h"


namespace Math
{

template<typename T>
requires(Core::IsArithmetic<T>)
[[nodiscard]] constexpr T log2(T n)
{
    if constexpr(Core::IsInteger<T>)
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
        i64 n_convert = i64(Math::floor(n));
        while (n_convert >>= 1)
        {
            result++;
        }
        return result;
    }
}

}