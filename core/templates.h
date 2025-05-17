#pragma once

// Type comparision
template<typename A, typename B>
inline constexpr bool IsSame = false;

template<typename A>
inline constexpr bool IsSame<A, A> = true;

template<typename T, typename... TArgs>
inline constexpr bool IsAnyOf = (IsSame<T, TArgs> || ...);

// Type modification

template<typename T>
struct RemoveConstT
{
    using Type = T;
};

template<typename T>
struct RemoveConstT<const T>
{
    using Type = T;
};

template<typename T>
using RemoveConst = typename RemoveConstT<T>::Type;


template<typename T>
struct RemoveVolatileT
{
    using Type = T;
};

template<typename T>
struct RemoveVolatileT<volatile T>
{
    using Type = T;
};

template<typename T>
using RemoveVolatile = typename RemoveVolatileT<T>::Type;

template<typename T>
using RemoveConstVolatile = RemoveConst<RemoveVolatile<T>>;

template<typename T>
struct RemoveReferenceT
{
    using Type = T;
};

template<typename T>
struct RemoveReferenceT<T&>
{
    using Type = T;
};

template<typename T>
struct RemoveReferenceT<T&&>
{
    using Type = T;
};

template<typename T>
using RemoveReference = typename RemoveReferenceT<T>::Type;

template<typename T>
struct RemovePointerT
{
    using Type = T;
};

template<typename T>
struct RemovePointerT<T*>
{
    using Type = T;
};

template<typename T>
struct RemovePointerT<const T*>
{
    using Type = const T;
};

template<typename T>
using RemovePointer = typename RemovePointerT<T>::Type;

template<typename T>
using RemoveConstPointer = RemoveConst<RemovePointer<RemoveReference<T>>>;

// Type checking

template<typename T>
inline constexpr bool IsInteger = IsAnyOf<
    RemoveConstVolatile<T>, 
    bool, char, signed char, unsigned char, wchar_t, 
    char8_t, char16_t, char32_t,
    short, unsigned short, int, unsigned int,
    long, unsigned long, long long, unsigned long long
>;

template<typename T>
inline constexpr bool IsFloatingPoint = IsAnyOf<
    T,
    float, double, long double
>;

template<typename T>
inline constexpr bool IsArithmetic = 
    IsInteger<T> || IsFloatingPoint<T>;

template<typename T>
inline constexpr bool IsConst = IsAnyOf<T, const T, const T*, const T*>;

