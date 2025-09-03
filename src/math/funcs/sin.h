#pragma once
#include "core/macros.h"
#include "core/templates.h"

namespace math::impl
{

// I don't know how this works.
template<typename T>
constexpr T sin_approx(T x)
{
    static_assert(
        IsArithmetic<T>,
        "expected arithmetic type"
    );

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
    result += term;
    term *= -x2 / (T(10.0) * T(11.0));   // -x^11 / 11!
    result += term;
    term *= -x2 / (T(12.0) * T(13.0));   // +x^13 / 13!
    result += term;

    return result;
}

}
