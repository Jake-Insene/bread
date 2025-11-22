#pragma once
#include "core/header.h"
#include "mem/allocator.h"


template<typename T>
concept CanBeCreatedWithAllocator = requires(const mem::Allocator& allocator)
{
	T::with_allocator(allocator);
};

template<typename T>
concept CanBeCreatedWithSize = requires(const mem::Allocator& allocator, usize size)
{
	T::with_size(allocator, size);
};

template<typename T, typename... TArgs>
concept CanBeCreated = requires(TArgs&&... args)
{
	T::create(Forward<TArgs>(args)...);
};

template<typename T>
concept CanBeDestroyed = requires(T& v)
{
	v.destroy();
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

	Scoped(const mem::Allocator& allocator) requires(CanBeCreatedWithAllocator<T>)
		: T(T::with_allocator(allocator)) {}

	Scoped(const mem::Allocator& allocator, usize size) requires(CanBeCreatedWithSize<T>)
		: T(T::with_size(allocator, size))
	{}

	Scoped(T scoped_value) : T(scoped_value) {}
	
	~Scoped() { T::destroy(); }
};