#pragma once
#include "core/macros.h"
#include "core/templates.h"

namespace math::impl
{

// I don't know how this works.
template<typename T>
constexpr T sin_approx(T r)
{
    static_assert(
        IsArithmetic<T>,
        "expected arithmetic type"
    );

    T r2 = r * r;
    T term = r;
    T result = term;

    term *= -r2 / (2.0 * 3.0);     // -x^3 / 3!
    result += term;
    term *= -r2 / (4.0 * 5.0);     // +x^5 / 5!
    result += term;
    term *= -r2 / (6.0 * 7.0);     // -x^7 / 7!
    result += term;
    term *= -r2 / (8.0 * 9.0);     // +x^9 / 9!
    result += term;
    term *= -r2 / (10.0 * 11.0);   // -x^11 / 11!
    result += term;
    term *= -r2 / (12.0 * 13.0);   // +x^13 / 13!
    result += term;

    return result;
}

}
