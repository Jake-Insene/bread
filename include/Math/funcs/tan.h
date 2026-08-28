#pragma once
#include "Math/funcs/sin.h"

namespace Math::impl
{

// I don't know how this works.
template<typename T>
requires(Core::IsArithmetic<T>)
T tan_approx(T x)
{
    return sin_approx(x);
}

}