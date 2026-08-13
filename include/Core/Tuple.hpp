#pragma once
#include "Core/Templates.hpp"


namespace Core
{

template<typename... TArgs>
struct Tuple {};

template<typename... TArgs>
Tuple(TArgs&&...) -> Tuple<TArgs...>;

template<typename T>
inline constexpr usize TupleSize = 0;

template<typename... TArgs>
inline constexpr usize TupleSize<Tuple<TArgs...>> = GetArgumentCount<TArgs...>();

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
    constexpr decltype(auto) get() const
    {
        if constexpr(Index == 0)
        {
            return value;
        }
        else
        {
            return Base::template get<Index - 1>();
        }
    }
};


template<typename T, typename TupleT, usize... Seq>
constexpr decltype(auto) FromTupleImpl(TupleT&& tuple, Sequence<Seq...>)
{
    return T(Forward<TupleT>(tuple).template get<Seq>()...);
}

template<typename T, typename TupleT>
constexpr decltype(auto) FromTuple(TupleT&& tuple)
{
    static constexpr usize ArgCount = TupleSize<RemoveConst<RemoveReference<TupleT>>>;
    using Seq = BuildSequence<ArgCount>;

    return FromTupleImpl<T>(Forward<TupleT>(tuple), Seq());
}

template<typename Fn, typename TupleT, usize... Seq>
constexpr decltype(auto) ApplyImpl(Fn&& fn, TupleT&& tuple, Sequence<Seq...>)
{
    return Forward<Fn>(fn)(Forward<TupleT>(tuple).template get<Seq>()...);
}

template<typename Fn, typename TupleT>
constexpr decltype(auto) Apply(Fn&& fn, TupleT&& tuple)
{
    static constexpr usize ArgCount = TupleSize<RemoveConst<RemoveReference<TupleT>>>;
    using Seq = BuildSequence<ArgCount>;

    return ApplyImpl(Forward<Fn>(fn), Forward<TupleT>(tuple), Seq());
}

template<typename Fn, typename T, typename TupleT, usize... Seq>
constexpr decltype(auto) ApplyMemberImpl(Fn&& fn, T* instance, TupleT&& tuple, Sequence<Seq...>)
{
    return (instance->*fn)(tuple.template get<Seq>()...);
}

template<typename Fn, typename T, typename TupleT>
constexpr decltype(auto) ApplyMember(Fn&& fn, T* instance, TupleT&& tuple)
{
    static constexpr usize ArgCount = TupleSize<RemoveConst<RemoveReference<TupleT>>>;
    using Seq = BuildSequence<ArgCount>;

    return ApplyMemberImpl(Forward<Fn>(fn), instance, Forward<Tuple>(tuple), Seq());
}

}
