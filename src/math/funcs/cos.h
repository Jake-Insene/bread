#pragma once
#include "core/macros.h"
#include "core/templates.h"
#include "math/constants.h"

namespace math::impl
{

// I don't know how this works.
template<typename T>
constexpr T cos_approx(T x)
{
    static_assert(
        IsArithmetic<T>,
        "expected arithmetic type"
    );

    while (x > PI<T>)  x -= 2 * PI<T>;
    while (x < -PI<T>) x += 2 * PI<T>;

    T x2 = x * x;

    // Taylor series expansion (first 5 terms)
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

}