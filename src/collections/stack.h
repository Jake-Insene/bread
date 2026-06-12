#pragma once
#include "core/types.h"
#include "mem/allocator.h"
#include "mem/utils.h"
#include "math/funcs.h"


template<typename T>
struct [[nodiscard]] Stack
{
	using Type = T;

	static constexpr usize DefaultCapacity = 4;

	Mem::Allocator* allocator;
	Slice<Type> items;
	usize sp;

	static Stack with_allocator(Mem::Allocator* allocator)
	{
		return Stack
		{
			.allocator = allocator,
			.items = allocator->array<Type>(DefaultCapacity),
			.sp = 0,
		};
	}

	static Stack with_size(Mem::Allocator* allocator, usize size)
	{
		return Stack
		{
			.allocator = allocator,
			.items = allocator->array<Type>(size),
			.sp = 0,
		};
	}


	void destroy()
	{
		if (items.ptr())
		{
			allocator->free(Mem::to_bytes(items));
		}
	}

	// funcs

	[[nodiscard]] bool is_empty() const { return sp == 0; }

	void ensure_capacity(usize required_capacity)
	{
		if (items.len >= required_capacity)
		{
			return;
		}

		usize new_cap = Math::min(items.len + (items.len / 2), required_capacity);

		if (!allocator->realloc(Mem::to_bytes(items), sizeof(Type) * new_cap, alignof(Type)))
		{
			Slice new_items = allocator->array<Type>(new_cap);
			if (items.ptr())
			{
				Mem::copy(new_items, items);
				allocator->free(Mem::to_bytes(items));
			}

			items = new_items;
		}
		else
		{
			items.len = new_cap;
			Slice items_to_construct = items.add(sp);
			ConstructArray(items_to_construct.ptr(), items_to_construct.len);
		}
	}

	void push(const Type& new_item)
	{
		ensure_capacity(sp + 1);
		items[sp++] = new_item;
	}

	Type pop()
	{
		DebugAssert(sp > 0, "stack is empty!");

		const Type item = items[sp - 1];
		if (sp > 1)
		{
			items[sp - 1] = Type();
		}

		sp--;
		return item;
	}
};


