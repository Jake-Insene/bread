#pragma once
#include "core/templates.h"


namespace Math::impl
{

// TODO:
// I don't know how this works.
// https://stackoverflow.com/questions/3380628/fast-arc-cos-algorithm
template<typename T>
requires(IsArithmetic<T>)
constexpr T acos_approx(T x)
{
    return (((T(-0.69813170079773212) * x * x) - T(0.87266462599716477)) * x) + T(1.5707963267948966);
}


}