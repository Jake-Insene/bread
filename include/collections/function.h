#pragma once
#include "Core/Header.h"
#include "debug/assertion.h"


template<typename Fn, bool UseInstance>
struct FunctionStorage
{
	Fn func;
};

template<typename RT, typename T, typename... TArgs>
struct FunctionStorage<RT(T::*)(TArgs...), true>
{
	T* instance;
	RT(T::*func)(TArgs...);
};

template<typename RT, typename T, typename... TArgs>
struct FunctionStorage<RT(T::*)(TArgs...), false>
{
	RT(T::*func)(TArgs...);
};

template<typename RT, typename T, typename... TArgs>
struct FunctionStorage<RT(T::*)(TArgs...) const, true>
{
	T* instance;
	RT(T::*func)(TArgs...) const;
};

template<typename RT, typename T, typename... TArgs>
struct FunctionStorage<RT(T::*)(TArgs...) const, false>
{
	RT(T::*func)(TArgs...) const;
};


template<typename Fn, bool UseInstance = true>
struct [[nodiscard]] Function
{
	using Decomposed = Core::FunctionDecomposed<Fn>;
	using ReturnType = Decomposed::ReturnType;

	FunctionStorage<Fn, UseInstance> storage;

	template<typename T>
	constexpr void bind(T func)
	requires(!Core::IsMemberFunction<Fn>)
	{
		storage.func = static_cast<Fn>(func);
	}

	template<typename T>
	constexpr void bind(T func)
	requires(!UseInstance)
	{
		storage.func = Fn(func);
	}

	template<typename T, typename Fn2>
	constexpr void bind(T* instance, Fn2 func)
	requires(Core::IsMemberFunction<Fn> && UseInstance)
	{
		storage.instance = reinterpret_cast<decltype(storage.instance)>(instance);
		storage.func = reinterpret_cast<Fn>(func);
	}

	template<typename... TArgs>
	constexpr ReturnType call(TArgs&&... args) const
	{
		DebugAssert(storage.func != nullptr, "function pointer don't set");
		if constexpr(Core::IsMemberFunction<Fn> && UseInstance)
		{
			DebugAssert(storage.instance != nullptr, "instance pointer don't set");
		}

		if constexpr(Core::IsMemberFunction<Fn> && UseInstance)
		{
			return (storage.instance->*storage.func)(args...);
		}
		else if constexpr(Core::IsMemberFunction<Fn> && !UseInstance)
		{
			return _call_method<Decomposed>(args...);
		}
		else
		{
			return storage.func(args...);
		}
	}

	template<typename DecomposedFn, typename... TArgs>
	requires(Core::IsMemberFunction<Fn>)
	constexpr ReturnType _call_method(DecomposedFn::ObjectType* instance, TArgs&&... args) const
	{
		return (instance->*storage.func)(args...);
	}

	constexpr bool has_func() const { return storage.func != nullptr; }
};

