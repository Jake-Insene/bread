#pragma once
#include "core/header.h"
#include "mem/allocator.h"


template<typename T>
concept CanBeDestroyed = requires(T& v)
{
	v.destroy();
};

template<typename T, typename... TArgs>
concept CanBeCreated = requires(TArgs&&... args)
{
	T::create(Forward<TArgs>(args)...);
};

template<typename T>
	requires(CanBeDestroyed<T>)
struct [[nodiscard]] Scoped : T
{
	Scoped(const Scoped&) = delete;
	Scoped(Scoped&&) = delete;

	template<typename... TArgs>
	Scoped(TArgs&&... args) requires(CanBeCreated<T, TArgs...>)
	: T(T::create(Forward<TArgs>(args)...)) {}

	Scoped(const mem::Allocator& allocator)
		: T(T::with_allocator(allocator)) {}

	Scoped(T scoped_value) : T(scoped_value) {}
	
	~Scoped() { T::destroy(); }
};