#pragma once
#include "core/templates.h"


template<typename... TArgs>
struct Tuple {};

template<>
struct [[nodiscard]] Tuple<>
{
};

template<typename T, typename... TArgs>
struct [[nodiscard]] Tuple<T, TArgs...> : Tuple<TArgs...>
{
	using Type = T;
	using Base = Tuple<TArgs...>;
	T value;

	constexpr Tuple(T first, TArgs... args)
		: Base(args...), value(first)
	{}

    template<usize Index>
    constexpr decltype(auto) get() const
    {
        if constexpr (Index == 0)
        {
            return value;
        }
        else
        {
            return Base::template get<Index - 1>();
        }
    }
};


