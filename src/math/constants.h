#pragma once
#include "core/macros.h"
#include "core/templates.h"

namespace math
{

template<typename T>
inline constexpr T PI = T(3.1415926535);

template<typename T>
inline constexpr T PI2 = T(2.0) * PI<T>;

template<typename T>
inline constexpr T PI_HALF = PI<T> * T(0.5);

template<typename T>
inline constexpr T ONE_PI = T(1.0) / PI<T>;

template<typename T>
inline constexpr T ONE_PI2 = T(1.0) / PI2<T>;

}