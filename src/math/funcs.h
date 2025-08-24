#pragma once
#include "core/templates.h"
#include "core/macros.h"

#include "math/funcs/abs.h"
#include "math/funcs/sin.h"
#include "math/funcs/cos.h"
#include "math/funcs/tan.h"
#include "math/funcs/atan2.h"

namespace math
{

template<typename T>
[[nodiscard]] constexpr T dist(T a, T b)
{
    return abs(b - a);
}

template<typename T>
[[nodiscard]] constexpr T log2(T n)
{
    static_assert(
        IsArithmetic<T>,
        "expected arithmetic type"
        );

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
    }
}

template<typename T>
[[nodiscard]] constexpr T next_pow2(const T n)
{
    static_assert(
        IsArithmetic<T>,
        "expected arithmetic type"
        );

    if (n == 0)
    {
        return 1;
    }

    T p = 1;
    while (p < n)
    {
        p *= 2;
    }

    return p;
}

template<typename T>
[[nodiscard]] constexpr T sqrt(T n)
{
    static_assert(IsFloatingPoint<T>, "expected floating point type");
    // TODO: Improve this
    if (n == T(0))
        return n;

    const T tolerance = T(1e-10);
    T x = n;

    while (true)
    {
        const T root = T(0.5) * (x + (n / x));
        
        if (abs(root - x) < tolerance)
            return root;

        x = root;
    }
}

template<typename T>
[[nodiscard]] constexpr T rads(const T degrees)
{
    static_assert(
        IsArithmetic<T>,
        "expected arithmetic type"
    );
    return (PI<T> / 180) * degrees;
}

template<typename T>
[[nodiscard]] constexpr T degrees(const T rads)
{
    static_assert(
        IsArithmetic<T>,
        "expected arithmetic type"
        );
    return (180 / PI<T>) * rads;

}

template<typename T>
[[nodiscard]] constexpr T cos(T r)
{
    return impl::cos_approx<T>(r);
}

template<typename T>
[[nodiscard]] constexpr T sin(T r)
{
    return impl::sin_approx<T>(r);
}

template<typename T>
[[nodiscard]] constexpr T tan(T r)
{
    return impl::tan_approx<T>(r);
}

template<typename T>
[[nodiscard]] constexpr T atan2(T y, T x)
{
    return impl::atan2_approx<T>(y, x);
}

template<typename T>
[[nodiscard]] constexpr T max(T a, T b)
{
    return a > b ? a : b;
}

template<typename T>
[[nodiscard]] constexpr T min(T a, T b)
{
    return a < b ? a : b;
}

template<typename T>
[[nodiscard]] constexpr T sign(T v)
{
    return v > 0 ? 1 : v < 0 ? -1 : 0;
}

template<typename T, typename TStep>
[[nodiscard]] constexpr T lerp(const T start, const T end, const TStep step)
{
    return start + (end - start) * step;
}

template<typename T, typename TStep>
[[nodiscard]] constexpr T move_to(const T start, const T end, TStep step)
{
    return dist<T>(start, end) <= step ? end : start + sign(end - start) * step;
}

template<typename T>
[[nodiscard]] constexpr i64 floor(T value)
{
    using Integer = i64;
    
    static_assert(
        IsArithmetic<T>,
        "expected arithmetic type"
    );


    const Integer i = static_cast<T>(value);
    return (value < 0 && value != static_cast<T>(i)) ? (i - 1) : i;
}

}
