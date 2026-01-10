#pragma once
#include "core/types.h"
#include "core/templates.h"


namespace math
{

template<typename T, typename Integer = Conditional<IsSame<T, f32>, i32, i64>>
    requires(IsFloatingPoint<T>)
[[nodiscard]] constexpr Integer floor(T value)
{
    const Integer i = Integer(value);
    return (value < 0 && value != static_cast<T>(i)) ? (i - 1) : i;
}

}