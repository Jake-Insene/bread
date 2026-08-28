#pragma once
#include "Core/Header.hpp"


namespace Math
{

template<typename T>
inline constexpr T PI = T(3.1415926535);

template<typename T>
inline constexpr T PI2 = T(2.0) * PI<T>;

template<typename T>
inline constexpr T PIOverTwo = PI<T> * T(0.5);

template<typename T>
inline constexpr T OneOverPI = T(1.0) / PI<T>;

template<typename T>
inline constexpr T OneOverPI2 = T(1.0) / PI2<T>;

}