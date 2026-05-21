#pragma once
#include "math/funcs/sin.h"
#include "math/funcs/cos.h"
#include "platform/instrinsics.h"


namespace Math::impl
{

template<typename T>
    requires(IsArithmetic<T>)
constexpr void sincos(T& s, T& c, T r)
{
    s = sin<T>(r);
    c = cos<T>(r);
}

}
