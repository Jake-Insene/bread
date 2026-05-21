#pragma once
#include "core/templates.h"
#include "core/macros.h"
#include "core/test.h"

#include "math/funcs/abs.h"
#include "math/funcs/acos.h"
#include "math/funcs/atan2.h"
#include "math/funcs/cos.h"
#include "math/funcs/floor.h"
#include "math/funcs/log.h"
#include "math/funcs/pow.h"
#include "math/funcs/sin.h"
#include "math/funcs/sincos.h"
#include "math/funcs/sqrt.h"
#include "math/funcs/tan.h"

#include <cmath>

namespace Math
{

template<typename Result, typename T>
requires(IsInteger<Result> && IsArithmetic<T>)
[[nodiscard]] constexpr Result trunc(T value)
{
    return Result(value);
}

template<typename T>
requires(IsArithmetic<T>)
[[nodiscard]] constexpr T ceil(T x)
{
    i64 xi = i64(x);

    if (x == xi)
    {
        return T(xi);
    }
    else if (x < 0)
    {
        return T(xi);
    }
    else
    {
        return T(xi + 1);
    }
}

template<typename T>
requires(IsArithmetic<T>)
[[nodiscard]] constexpr T sign(T v)
{
    return v > 0 ? T(1) : v < 0 ? T(-1) : T(0);
}

template<typename T>
requires(IsArithmetic<T>)
[[nodiscard]] constexpr T clamp(T value, T min, T max)
{
    return value < min ? min
        : value > max ? max
        : value;
}

template<typename T>
requires(IsArithmetic<T>)
[[nodiscard]] constexpr T dist(T a, T b)
{
    return ::Math::abs(b - a);
}

template<typename T>
requires(IsArithmetic<T>)
[[nodiscard]] constexpr T next_pow2(const T n)
{
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


template<typename T, typename... TArgs>
requires(IsArithmetic<T> && (IsArithmetic<TArgs> && ...))
[[nodiscard]] constexpr T min(T a, TArgs... args)
{
    if constexpr (sizeof...(args) == 0)
    {
        return a;
    }
    else
    {
        return a < min<T>(args...) ? a : min<T>(args...);
    }
}

template<typename T, typename... TArgs>
requires(IsArithmetic<T> && (IsArithmetic<TArgs> && ...))
[[nodiscard]] constexpr T max(T a, TArgs... args)
{
    if constexpr (sizeof...(args) == 0)
    {
        return a;
    }
    else
    {
        return a > max<T>(args...) ? a : max<T>(args...);
    }
}

template<typename T>
requires(IsArithmetic<T>)
[[nodiscard]] constexpr T lerp(const T start, const T end, const T step)
{
    return start + (end - start) * step;
}

template<typename T>
[[nodiscard]] constexpr T move_to(const T start, const T end, T step)
requires(IsArithmetic<T>)
{
    return dist<T>(start, end) <= step ? end : start + sign(end - start) * step;
}

// Mod

template<typename T>
requires(IsArithmetic<T>)
[[nodiscard]] constexpr T mod(T a, T b)
{
    if constexpr (IsInteger<T>)
    {
        return a % b;
    }
    else
    {
        return a - trunc<i64>(a / b) * b;
    }
}

// Angles

template<typename T>
requires(IsArithmetic<T>)
[[nodiscard]] constexpr T rads(const T degrees)
{
    return (PI<T> / T(180)) * degrees;
}

template<typename T>
requires(IsArithmetic<T>)
[[nodiscard]] constexpr T degrees(const T rads)
{
    return (T(180) / PI<T>) * rads;
}

/*
* @param r Angle in radians, compute the sine(x)
*/
template<typename T>
requires(IsArithmetic<T>)
[[nodiscard]] constexpr T sin(T r)
{
    return impl::sin<T>(r);
}

/*
* @param r Angle in radians, compute the cosine(x)
*/
template<typename T>
requires(IsArithmetic<T>)
[[nodiscard]] constexpr T cos(T r)
{
    return impl::cos<T>(r);
}

template<typename T>
requires(IsArithmetic<T>)
constexpr void sincos(T& s, T& c, T r)
{
    impl::sincos<T>(s, c, r);
}

/*
* @param r Angle in radians, compute the tan(x)
*/
template<typename T>
requires(IsArithmetic<T>)
[[nodiscard]] constexpr T tan(T r)
{
    return impl::tan_approx<T>(r);
}

/*
* @param r Angle in radians, compute the acos(x)
*/
template<typename T>
requires(IsArithmetic<T>)
[[nodiscard]] constexpr T acos(T r)
{
    return impl::acos_approx(r);
}

template<typename T>
requires(IsArithmetic<T>)
[[nodiscard]] constexpr T atan2(T y, T x)
{
    return impl::atan2_approx<T>(y, x);
}

}


