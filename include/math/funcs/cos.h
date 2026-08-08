#pragma once
#include "math/constants.h"


namespace Math::impl
{

/*
* Reference: https://www.mathsisfun.com/algebra/taylor-series.html
*/
template<typename T>
requires(Core::IsArithmetic<T>)
constexpr T cos_approx(T x)
{
    // Using taylor series for only 5 terms,
    // x is converted to a value between [-PI/2, PI/2].

    while (x > PI<T>)  x -= 2 * PI<T>;
    while (x < -PI<T>) x += 2 * PI<T>;

    T x2 = x * x;
    T result = T(1.0);
    T term = T(1.0);

    term *= -x2 / T(2.0);     // -x^2 / 2!
    result += term;

    term *= -x2 / T(3.0 * 4.0); // +x^4 / 4!
    result += term;

    term *= -x2 / T(5.0 * 6.0); // -x^6 / 6!
    result += term;

    term *= -x2 / T(7.0 * 8.0); // +x^8 / 8!
    result += term;

    return result;
}

template<typename T>
requires(Core::IsArithmetic<T>)
constexpr T cos(T rads)
{
    return cos_approx<T>(rads);
}

}