#pragma once
#include "Core/Mem/Mem.hpp"
#include "Core/Templates.hpp"


namespace Core
{

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
* @brief User to represent comparable values of type T.
*/
template<typename T>
concept Comparable = requires(T&& value1, T&& value2)
{
    { Comparator<T>::compare(value1, value2) } -> Core::ConvertibleTo<bool>;
};

template<typename T, typename... TArgs>
concept ConstructibleFrom = requires(T& object, TArgs&&... args)
{
    { Core::Mem::Placement(object, Core::Forward<TArgs>(args)...) } -> Core::ConvertibleTo<void>;
};

template<typename T>
concept Destructible = requires(T& object)
{
    { Core::Mem::Destruct(object) } -> Core::ConvertibleTo<void>;
};

template<typename L, typename R>
concept AssignableFrom = requires(L left, R&& right)
{
    { left = Core::Forward<R>(right) } -> ConvertibleTo<L>;
};

template<typename T>
concept Copyable = ConstructibleFrom<T, const T&> && AssignableFrom<T&, const T&>;

template<typename T>
concept Movable = ConstructibleFrom<T, T&&> && AssignableFrom<T&, T>;

}
