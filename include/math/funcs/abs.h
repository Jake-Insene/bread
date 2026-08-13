#pragma once
#include "Core/Templates.hpp"


namespace Math
{

template<typename T>
requires(Core::IsArithmetic<T>)
constexpr T abs(T a)
{
    return a >= 0 ? a : -a;
}


}