#pragma once
#include "Math/funcs/sin.h"
#include "Math/funcs/cos.h"
#include "Platform/instrinsics.h"


namespace Math::impl
{

template<typename T>
requires(Core::IsArithmetic<T>)
constexpr void sincos(T* s, T* c, T rads)
{
    *s = sin<T>(rads);
    *c = cos<T>(rads);
}

}
