#pragma once
#include "core/templates.h"


namespace math
{

template<typename T>
constexpr T __log(T x)
{
    if (x <= 0) 
        return 0; // domain error -> return 0
    i32 n = 0;
    while (x > 2) { x *= T(0.5); n++; }
    while (x < T(0.5)) { x *= T(2.0); n--; }

    // Use series expansion around 1.0: log(1+z) ~ z - z^2/2 + z^3/3
    T z = x - T(1.0);
    T z2 = z * z;
    T z3 = z2 * z;
    T approx = z - z2 * T(0.5) + z3 * T(1.0 / 3.0);

    return approx + n * T(0.6931471805599453); // ln(2)
}

template<typename T>
constexpr T __exp(T x)
{
    // Crude exp approximation using series exp(x) ~ 1 + x + x^2/2 + x^3/6 + x^4/24
    T term = T(1.0);
    T sum = T(1.0);
    for (i32 i = 1; i < 12; i++)
    {
        term *= x / i;
        sum += term;
    }
    return sum;
}

template<typename FP>
constexpr i64 __pow_integer(FP base, i64 exp)
{
    FP result = FP(1.0);
    FP b = base;
    i64 e = exp;
    if (e < 0)
    {
        e = -e; 
        b = FP(1.0) / b;
    }
    while (e)
    {
        if (e & 1)
            result *= b;
        b *= b;
        e >>= 1;
    }

    return i64(result);
}

template<typename T>
    requires(IsArithmetic<T>)
[[nodiscard]] constexpr T pow(T x, T y)
{
    if constexpr (IsInteger<T>)
    {
        return T(__pow_integer<f64>(f64(x), i64(y)));
    }
    else
    {
        // handle integer exponent fast
        i64 yi = i64(y);
        if (T(yi) == y)
        {
            return T(__pow_integer<T>(x, yi));
        }
        // general fallback: x^y = exp(y * ln(x))
        
        // no negative base support without CRT
        if (x <= 0) 
            return 0;
        return __exp<T>(y * __log<T>(x));
    }
}

}