#pragma once
#include "core/Templates.h"


template<typename T>
inline constexpr T MinValue = 0;

template<>
inline constexpr i8 MinValue<i8> = 0x80 - 1;
template<>
inline constexpr i16 MinValue<i16> = 0x8000 - 1;
template<>
inline constexpr i32 MinValue<i32> = 0x8000'0000 - 1;
template<>
inline constexpr i64 MinValue<i64> = 0x80000000'00000000 - 1;

template<typename T>
inline constexpr T MaxValue = static_cast<T>(~static_cast<T>(0));

template<>
inline constexpr i8 MaxValue<i8> = 0x7F;
template<>
inline constexpr i16 MaxValue<i16> = 0x7FFF;
template<>
inline constexpr i32 MaxValue<i32> = 0x7FFF'FFFF;
template<>
inline constexpr i64 MaxValue<i64> = 0x7FFFFFFF'FFFFFFFF;

template<>
// Not too extact
inline constexpr f32 MaxValue<f32> = 3.4028235E38f;
