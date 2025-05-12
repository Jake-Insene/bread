#pragma once
#include "core/macros.h"
#include "core/templates.h"


#include <concepts> 

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
struct EventFnDecomposed;

template<typename RT, typename... TArgs>
struct EventFnDecomposed<RT(*)(TArgs...)>
{
	using ReturnType = RT;
	using ObjectType = RT*;
};

template<typename RT, typename T, typename... TArgs>
struct EventFnDecomposed<RT(T::*)(TArgs...)>
{
	static constexpr bool IsConst = false;
	using ReturnType = RT;
	using ObjectType = T;
};

template<typename RT, typename T, typename... TArgs>
struct EventFnDecomposed<RT(T::*)(TArgs...) const>
{
	static constexpr bool IsConst = true;
	using ReturnType = RT;
	using ObjectType = T;
};

template<typename Fn>
struct [[nodiscard]] Event
{
	using Decomposed = EventFnDecomposed<Fn>;

	using ReturnType = Decomposed::ReturnType;

	Event(Fn fn = nullptr) : func(fn) {}
	
	Event& operator=(Fn fn)
	{
		func = fn;
		return *this;
	}

	Fn func;

	template<typename T>
	constexpr void bind(T fn)
	{
		func = (Fn)fn;
	}

	template<typename... TArgs>
	constexpr ReturnType call(TArgs&&... args) const
	{
		if constexpr (IsSame<ReturnType, void>)
		{
			if constexpr (IsMemberFunction<Fn>)
			{
				_call_method(args...);
			}
			else
			{
				func(args...);
			}
		}
		else
		{
			if constexpr (IsMemberFunction<Fn>)
			{
				return _call_method(args...);
			}
			else
			{
				return func(args...);
			}
		}
	}

	template<typename... TArgs>
	constexpr ReturnType _call_method(EventFnDecomposed<Fn>::ObjectType*& instance, TArgs&&... args) const
	{
		if constexpr (IsSame<ReturnType, void>)
		{
			(instance->*func)(args...);
		}
		else
		{
			return (instance->*func)(args...);
		}
	}

	constexpr bool has_func() const { return func != nullptr; }
};

