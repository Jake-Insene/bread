#pragma once
#include "core/Templates.h"


template<typename T>
concept Iterable = requires(T&& it)
{
	{ it.begin() };
	{ it.end() };
	{ ++it };
	{ --it };
	{ *it };
};

// Default hash
using HashCode = u64;

template<typename T>
struct HashOfType
{
    // Default
    [[nodiscard]] static constexpr HashCode hashfunc(const T& k) { return HashCode(k); }
};

template<typename T>
concept Hashable = requires(T&& hashable1)
{
    { HashOfType<T>::hashfunc(hashable1) } -> Core::ConvertibleTo<HashCode>;
};

template<typename T>
struct Comparator
{
    [[nodiscard]] static constexpr bool compare(const T& value1, const T& value2)
    {
        // Default    
        return value1 == value2;
    }
};

/**
* User to represent comparable values of type T.
*/
template<typename T>
concept Comparable = requires(T&& value1, T&& value2)
{
    { Comparator<T>::compare(value1, value2) } -> Core::ConvertibleTo<bool>;
};
