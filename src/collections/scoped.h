#pragma once
#include "collections/tuple.h"
#include "mem/allocator.h"


template<typename T>
struct Ptr;


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

template<typename T, typename... TArgs>
concept CanBeDestroyed = requires(T& v, TArgs&&... args)
{
	v.destroy(Forward<TArgs>(args)...);
};

template <typename T, typename List>
struct CanBeDestroyedFromListT { static constexpr bool value = false; };

template <typename T, typename... Ts>
struct CanBeDestroyedFromListT<T, TypeList<Ts...>> {
    static constexpr bool value = CanBeDestroyed<T, Ts...>;
};

template<typename T, typename... TArgs>
inline constexpr bool CanBeDestroyedFromList = CanBeDestroyedFromListT<T, TArgs...>::value;


template<typename T>
struct ScopedData
{
	using DestroyArgList = TypeList<>;

	template<typename... TArgs>
	ScopedData(TArgs&&... args) { Unused(args...); }

	auto as_tuple() const { return Tuple<>(); }
};


template<typename T>
struct [[nodiscard]] Scoped : T
{
	ScopedData<T> data;

	Scoped(const Scoped&) = delete;
	Scoped(Scoped&&) = delete;

	template<typename... TArgs>
	Scoped(TArgs&&... args)
		requires(CanBeCreated<T, TArgs...>)
	: T(T::create(Forward<TArgs>(args)...)), data(Forward<TArgs>(args)...) {}

	Scoped(const mem::Allocator& allocator)
		requires(CanBeCreatedWithAllocator<T>)
	: T(T::with_allocator(allocator)), data(allocator) {}

	Scoped(const mem::Allocator& allocator, usize size)
		requires(CanBeCreatedWithSize<T>)
	: T(T::with_size(allocator, size)), data(allocator) {}

	Scoped(T scoped_value) : T(scoped_value), data() {}
	
	~Scoped()
	requires(CanBeDestroyedFromList<T, typename ScopedData<T>::DestroyArgList>)
	{
		ApplyMember(&T::destroy, this, data.as_tuple());
	}
};