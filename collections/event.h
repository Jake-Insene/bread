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
struct EventFnDecomposed {};

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


template<typename Fn, bool UseInstance>
struct EventStorage
{
	Fn func;
};


template<typename RT, typename T, typename... TArgs>
struct EventStorage<RT(T::*)(TArgs...), true>
{
	T* instance;
	RT(T::*func)(TArgs...);
};

template<typename RT, typename T, typename... TArgs>
struct EventStorage<RT(T::*)(TArgs...), false>
{
	RT(T::*func)(TArgs...);
};

template<typename RT, typename T, typename... TArgs>
struct EventStorage<RT(T::*)(TArgs...) const, true>
{
	T* instance;
	RT(T::*func)(TArgs...) const;
};

template<typename RT, typename T, typename... TArgs>
struct EventStorage<RT(T::*)(TArgs...) const, false>
{
	RT(T::*func)(TArgs...) const;
};


template<typename Fn, bool UseInstance = true>
struct [[nodiscard]] Event
{
	using Decomposed = EventFnDecomposed<Fn>;
	using ReturnType = Decomposed::ReturnType;

	EventStorage<Fn, UseInstance> storage{};

	template<typename T>
	constexpr void bind(T func)
		requires(!IsMemberFunction<Fn>)
	{
		storage.func = (Fn)func;
	}

	template<typename T>
	constexpr void bind(T func)
		requires(!UseInstance)
	{
		storage.func = Fn(func);
	}

	template<typename T, typename Fn2>
	constexpr void bind(T* instance, Fn2 func)
		requires(IsMemberFunction<Fn> && UseInstance)
	{
		storage.instance = (decltype(storage.instance))instance;
		storage.func = (Fn)func;
	}

	template<typename... TArgs>
	constexpr ReturnType call(TArgs&&... args) const
	{
		DebugAssert(storage.func != nullptr, "function pointer don't set");
		if constexpr (IsMemberFunction<Fn> && UseInstance)
		{
			DebugAssert(storage.instance != nullptr, "instance pointer don't set");
		}

		if constexpr (IsSame<ReturnType, void>)
		{
			if constexpr (IsMemberFunction<Fn> && UseInstance)
			{
				(storage.instance->*storage.func)(args...);
			}
			else if constexpr (IsMemberFunction<Fn> && !UseInstance)
			{
				_call_method<Decomposed>(args...);
			}
			else
			{
				storage.func(args...);
			}
		}
		else
		{
			if constexpr (IsMemberFunction<Fn> && UseInstance)
			{
				(storage.instance->*storage.func)(args...);
			}
			else if constexpr (IsMemberFunction<Fn> && !UseInstance)
			{
				return _call_method<Decomposed>(args...);
			}
			else
			{
				return storage.func(args...);
			}
		}
	}

	template<typename DecomposedFn, typename... TArgs>
	requires(IsMemberFunction<Fn>)
	constexpr ReturnType _call_method(DecomposedFn::ObjectType* instance, TArgs&&... args) const
	{
		if constexpr (IsSame<ReturnType, void>)
		{
			(instance->*storage.func)(args...);
		}
		else
		{
			return (instance->*storage.func)(args...);
		}
	}

	constexpr bool has_func() const { return storage.func != nullptr; }
};

