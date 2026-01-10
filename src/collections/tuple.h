#pragma once
#include "core/templates.h"


template<typename... TArgs>
struct Tuple {};

template<>
struct [[nodiscard]] Tuple<>
{};

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
    constexpr auto get() const
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


template<typename Fn, typename... TArgs, usize... Seq>
constexpr auto ApplyImpl(Fn&& fn, Tuple<TArgs...>&& tuple, Sequence<Seq...>)
{
    return fn(tuple.template get<Seq>()...);
}


template<typename Fn, typename... TArgs>
constexpr auto Apply(Fn&& fn, Tuple<TArgs...>&& tuple)
{
    static constexpr usize ArgCount = GetArgumentCount<TArgs...>();
    using Seq = BuildSequence<ArgCount>;

    return ApplyImpl(Forward<Fn>(fn), Forward<Tuple<TArgs...>>(tuple), Seq());
}

template<typename Fn, typename T, typename... TArgs, usize... Seq>
constexpr auto ApplyMemberImpl(Fn&& fn, T* instance, Tuple<TArgs...>&& tuple, Sequence<Seq...>)
{
    return (instance->*fn)(tuple.template get<Seq>()...);
}

template<typename Fn, typename T, typename... TArgs>
constexpr auto ApplyMember(Fn&& fn, T* instance, Tuple<TArgs...>&& tuple)
{
    static constexpr usize ArgCount = GetArgumentCount<TArgs...>();
    using Seq = BuildSequence<ArgCount>;

    return ApplyMemberImpl(Forward<Fn>(fn), instance, Forward<Tuple<TArgs...>>(tuple), Seq());
}

