#pragma once
#include "Math/constants.h"
#include "Platform/instrinsics.h"


namespace Math::impl
{

/*
* Reference: https://www.mathsisfun.com/algebra/taylor-series.html
*/
template<typename T>
requires(Core::IsArithmetic<T>)
constexpr T sin_approx(T x)
{
    // Using taylor series for only 5 terms,
    // x is converted to a value between [-PI, PI].

    while (x > PI<T>)  x -= 2 * PI<T>;
    while (x < -PI<T>) x += 2 * PI<T>;

    T x2 = x * x;
    T term = x;
    T result = term;

    // 1/3!
    static constexpr T fact_1_3 = T(1) / (T(2) * T(3));
    // 1/5!
    static constexpr T fact_1_5 = T(1) / (T(4) * T(5));
    // 1/7!
    static constexpr T fact_1_7 = T(1) / (T(6) * T(7));
    // 1/9!
    static constexpr T fact_1_9 = T(1) / (T(8) * T(9));

    term *= -x2 * fact_1_3;     // -x^3 / 3!
    result += term;

    term *= -x2 * fact_1_5;     // +x^5 / 5!
    result += term;
    
    term *= -x2 * fact_1_7;     // -x^7 / 7!
    result += term;
    
    term *= -x2 * fact_1_9;     // +x^9 / 9!
    result += term;

    return result;
}


template<typename T>
requires(Core::IsArithmetic<T>)
constexpr T sin(T rads)
{
    return sin_approx<T>(rads);
}

}
