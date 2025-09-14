#pragma once
#include "core/macros.h"
#include "core/templates.h"

namespace math::impl
{

/*
* Reference: https://www.mathsisfun.com/algebra/taylor-series.html
*/
template<typename T>
constexpr T sin_approx(T x)
{
    static_assert(
        IsArithmetic<T>,
        "expected arithmetic type"
    );

    // Using taylor series for only 5 terms,
    // x is converted to a value between [-PI, PI].

    while (x > PI<T>)  x -= 2 * PI<T>;
    while (x < -PI<T>) x += 2 * PI<T>;

    T x2 = x * x;
    T term = x;
    T result = term;

    term *= -x2 / (T(2.0) * T(3.0));     // -x^3 / 3!
    result += term;
    term *= -x2 / (T(4.0) * T(5.0));     // +x^5 / 5!
    result += term;
    term *= -x2 / (T(6.0) * T(7.0));     // -x^7 / 7!
    result += term;
    term *= -x2 / (T(8.0) * T(9.0));     // +x^9 / 9!

    return result;
}

}
