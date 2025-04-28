#pragma once
#include "core/macros.h"
#include "core/types.h"


template<typename Fn>
constexpr bool IsMemberFunction = false;

template<typename RT, typename T, typename... TArgs>
constexpr bool IsMemberFunction<RT(T::*)(TArgs&&...)> = true;

template<typename Fn>
struct EventFnDecomposed;

template<typename RT, typename... TArgs>
struct EventFnDecomposed<RT(*)(TArgs&&...)>
{
	using RetType = RT;
};

template<typename RT, typename T, typename... TArgs>
struct EventFnDecomposed<RT(T::*)(TArgs&&...)>
{
	using RetType = RT;
	using ObjectType = T;
};

template<typename Fn>
struct [[nodiscard]] Event
{
	using Decomposed = EventFnDecomposed<Fn>;

	using ReturnType = Decomposed::RetType;

	Fn func;

	template<typename... TArgs>
	constexpr ReturnType call(TArgs&&... args)
	{
		if constexpr (IsMemberFunction<Fn>)
		{
			if constexpr (IsSame<ReturnType, void>)
			{
				call_method(args...);
			}
			else
			{
				return call_method(args...);
			}
		}
		else
		{
			if constexpr(IsSame<ReturnType, void>)
			{
				return func(args...);
			}
			else
			{
				func(args...);
			}
		}
	}

	template<typename... TArgs>
	constexpr ReturnType call_method(Decomposed::ObjectType& instance, TArgs&&... args)
	{
		if constexpr (IsSame<ReturnType, void>)
		{
			(instance.*func)(args...);
		}
		else
		{
			return (instance.*func)(args...);
		}
	}

	template<>
	constexpr ReturnType call_method<>(Decomposed::ObjectType& instance)
	{
		if constexpr (IsSame<ReturnType, void>)
		{
			(instance.*func)();
		}
		else
		{
			return (instance.*func)();
		}
	}

};

