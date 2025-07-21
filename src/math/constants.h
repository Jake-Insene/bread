#pragma once
#include "core/macros.h"
#include "core/templates.h"

namespace math
{

template<typename T>
constexpr T PI = T(3.1415926535);

template<typename T>
constexpr T PI2 = PI<T> / T(2);

}