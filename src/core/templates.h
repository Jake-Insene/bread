#pragma once
#include "core/types.h"


// Type comparison
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
inline constexpr bool IsSigned = IsAnyOf<
    RemoveConstVolatile<T>,
    char, signed char, wchar_t, 
    char8_t, char16_t, char32_t,
    short, int, long, long long
>;

template<typename T>
inline constexpr bool IsUnsigned = IsAnyOf<
    RemoveConstVolatile<T>,
    unsigned char, unsigned short, unsigned int, 
    unsigned long, unsigned long long
>;

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
inline constexpr bool IsConst = IsAnyOf<T, const T, T* const, const T*>;

template<typename T>
inline constexpr bool IsPointer = false;

template<typename T>
inline constexpr bool IsPointer<T*> = true;

template<typename T>
inline constexpr bool IsPointer<T* const> = true;

template<typename T>
inline constexpr bool IsPointer<T* volatile> = true;

template<typename T>
inline constexpr bool IsPointer<T* const volatile> = true;

// Value Checking
template<typename T, T Value1, T Value2>
inline constexpr bool IsSameValue = false;

template<typename T, T Value>
inline constexpr bool IsSameValue<T, Value, Value> = true;

template<typename T, T Value, T... Args>
inline constexpr bool IsAnyOfValue = (IsSameValue<T, Value, Args> || ...);


// Conditional Selection
template<bool Value, typename T1, typename T2>
struct ConditionalT
{
    using Type = T1;
};

template<typename T1, typename T2>
struct ConditionalT<false, T1, T2>
{
    using Type = T2;
};

template<bool Value, typename T1, typename T2>
using Conditional = ConditionalT<Value, T1, T2>::Type;

template<typename T, bool Cond, T ValueTrue, T ValueFalse>
struct ConditionalValueT
{
    static constexpr T Value = ValueTrue;
};

template<typename T, T ValueTrue, T ValueFalse>
struct ConditionalValueT<T, false, ValueTrue, ValueFalse>
{
    static constexpr T Value = ValueFalse;
};

template<typename T, bool Cond, T ValueTrue, T ValueFalse>
inline constexpr T ConditionalValue = ConditionalValueT<T, Cond, ValueTrue, ValueFalse>::Value;

template<typename T>
using MakeUnsigned = Conditional<sizeof(T) == 1, u8, Conditional<sizeof(T) == 2, u16, Conditional<sizeof(T) == 4, u32, u64>>>;


template<typename T>
struct TypeIdentityT
{
    using Type = T;
};

template<typename T>
using TypeIdentity = TypeIdentityT<T>::Type;


template<typename Fn>
struct IsMemberFunctionT
{
    static constexpr bool Value = false;
};

template<typename RT, typename T, typename... TArgs>
struct IsMemberFunctionT<RT(T::*)(TArgs...)>
{
    static constexpr bool Value = true;
};

template<typename RT, typename T, typename... TArgs>
struct IsMemberFunctionT<RT(T::*)(TArgs...) const>
{
    static constexpr bool Value = true;
};

template<typename Fn>
inline constexpr bool IsMemberFunction = IsMemberFunctionT<Fn>::Value;

template<typename Fn>
struct FunctionDecomposed;

template<typename RT, typename... TArgs>
struct FunctionDecomposed<RT(*)(TArgs...)>
{
    using ReturnType = RT;
};

template<typename RT, typename T, typename... TArgs>
struct FunctionDecomposed<RT(T::*)(TArgs...)>
{
    static constexpr bool IsConst = false;
    using ReturnType = RT;
    using ObjectType = T;
};

template<typename RT, typename T, typename... TArgs>
struct FunctionDecomposed<RT(T::*)(TArgs...) const>
{
    static constexpr bool IsConst = true;
    using ReturnType = RT;
    using ObjectType = T;
};

template<typename T>
struct FunctionDecomposed : FunctionDecomposed<decltype(&T::operator())> {};


template<typename... TArgs>
constexpr void Unused(TArgs...) {}

template<typename... TArgs>
constexpr usize GetArgumentCount()
{
    return sizeof...(TArgs);
}

template<usize N, typename T, typename... TArgs>
constexpr auto GetArgument(T first, TArgs... args)
{
    if constexpr (N == 0)
    {
        Unused(args...);
        return TypeIdentity<T>(first);
    }
    else
    {
        return GetArgument<N - 1, TArgs...>(args...);
    }
}

template<typename T>
[[nodiscard]] constexpr T&& Forward(RemoveReference<T>& arg)
{
    return arg;
}

template<typename T>
[[nodiscard]] constexpr T&& Forward(RemoveReference<T>&& arg)
{
    return arg;
}

