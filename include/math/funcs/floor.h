#pragma once
#include "Core/Templates.h"


namespace Math
{

template<typename T, typename Integer = Core::Conditional<Core::IsSame<T, f32>, i32, i64>>
requires(Core::IsFloatingPoint<T>)
[[nodiscard]] constexpr Integer floor(T value)
{
    const Integer i = Integer(value);
    return (value < 0 && value != static_cast<T>(i)) ? (i - 1) : i;
}

}