#pragma once
#include "core/types.h"
#include "core/templates.h"

namespace math
{

template<typename T>
[[nodiscard]] constexpr i64 floor(T value)
{
    static_assert(IsArithmetic<T>, "expected arithmetic type");
    using Integer = i64;
    const Integer i = Integer(value);
    return (value < 0 && value != static_cast<T>(i)) ? (i - 1) : i;
}

}