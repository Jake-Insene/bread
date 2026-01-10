#pragma once
#include "core/templates.h"


namespace math
{

template<typename T>
    requires(IsArithmetic<T>)
constexpr T abs(T a)
{
    return a >= 0 ? a : -a;
}


}