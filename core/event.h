#pragma once
#include "core/templates.h"


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
struct EventStorage
{
	Fn func;
};


template<typename RT, typename T, typename... TArgs>
struct EventStorage<RT(T::*)(TArgs...)>
{
	T* instance;
	RT(T::*func)(TArgs...);
};

template<typename RT, typename T, typename... TArgs>
struct EventStorage<RT(T::*)(TArgs...) const>
{
	T* instance;
	RT(T::*func)(TArgs...) const;
};


template<typename Fn>
struct [[nodiscard]] Event
{
	using Decomposed = EventFnDecomposed<Fn>;
	using ReturnType = Decomposed::ReturnType;

	EventStorage<Fn> storage{};

	template<typename = EnableIf<!IsMemberFunction<Fn>, int>, typename T, typename... TArgs>
	constexpr void bind(T func)
	{
		storage.func = (Fn)func;
	}

	template<typename = EnableIf<IsMemberFunction<Fn>, int>, typename T, typename Fn2>
	constexpr void bind(T* instance, Fn2 func)
	{
		storage.instance = (decltype(storage.instance))instance;
		storage.func = (Fn)func;
	}

	template<typename... TArgs>
	constexpr ReturnType call(TArgs&&... args) const
	{
		DebugAssert(storage.func != nullptr, "function pointer don't set");
		if constexpr (IsMemberFunction<Fn>)
		{
			DebugAssert(storage.instance != nullptr, "instance pointer don't set");
		}

		if constexpr (IsSame<ReturnType, void>)
		{
			if constexpr (IsMemberFunction<Fn>)
			{
				_call_method(args...);
			}
			else
			{
				storage.func(args...);
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
				return storage.func(args...);
			}
		}
	}

	template<typename... TArgs>
	constexpr ReturnType _call_method(TArgs&&... args) const
	{
		if constexpr (IsSame<ReturnType, void>)
		{
			(storage.instance->*storage.func)(args...);
		}
		else
		{
			return (storage.instance->*storage.func)(args...);
		}
	}

	constexpr bool has_func() const { return storage.func != nullptr; }
};

