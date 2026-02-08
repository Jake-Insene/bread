#pragma once
#include "core/types.h"
#include "mem/allocator.h"
#include "mem/utils.h"


template<typename T>
struct [[nodiscard]] Stack
{
	using Type = T;

	static constexpr usize DefaultCapacity = 4;

	mem::Allocator allocator;
	Slice<Type> items;
	usize sp;

	static Stack with_allocator(const mem::Allocator& allocator)
	{
		return Stack
		{
			.allocator = allocator,
			.items = allocator.array<Type>(DefaultCapacity),
			.sp = 0,
		};
	}

	static Stack with_size(const mem::Allocator& allocator, usize size)
	{
		return Stack
		{
			.allocator = allocator,
			.items = allocator.array<Type>(size),
			.sp = 0,
		};
	}


	void destroy()
	{
		if (items.ptr())
		{
			allocator.free(mem::to_bytes(items));
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

		usize new_cap = items.len + items.len / 2;
		if (new_cap < required_capacity)
		{
			new_cap = required_capacity;
		}

		if (!allocator.realloc(mem::to_bytes(items), sizeof(Type) * new_cap, alignof(Type)))
		{
			Slice<Type> new_items = allocator.array<Type>(new_cap);
			if (items.ptr())
			{
				mem::copy(new_items, items);
				allocator.free(mem::to_bytes(items));
			}

			items = new_items;
		}
		else
		{
			items.len = new_cap;
			allocator.construct_array(items.add(sp));
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


