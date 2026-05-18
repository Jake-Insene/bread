#pragma once
#include "core/templates.h"


template<typename T>
concept Iterable = requires(T&& it)
{
	{ it.begin() };
	{ it.end() };
};

// Default hash
using HashCode = u64;

template<typename T>
struct HashOfType
{
    [[nodiscard]] static constexpr HashCode hashfunc(const T& k) { return HashCode(k); }
    [[nodiscard]] static constexpr bool compare(const T& k1, const T& k2) { return k1 == k2; }
};

template<typename T>
concept Hashable = requires(T&& hashable1, T&& hashable2)
{
    { HashOfType<T>::hashfunc(hashable1) } -> ConvertibleTo<HashCode>;
    { HashOfType<T>::compare(hashable1, hashable2) } -> ConvertibleTo<bool>;
};

template<typename T>
concept Comparable = requires(T&& value1, T&& value2)
{
    { value1.equals(value2) } -> ConvertibleTo<bool>;
};
