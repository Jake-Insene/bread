#pragma once
#include "math/funcs/sin.h"
#include "math/funcs/cos.h"
#include "platform/instrinsics.h"


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
