#pragma once
#include "Core/Header.h"
#include "mem/allocator.h"
#include "mem/utils.h"
#include "math/funcs.h"


template<typename T>
struct [[nodiscard]] Stack
{
	DisableCopy(Stack);
	DisableMove(Stack);
	using Type = T;

	static constexpr usize DefaultCapacity = 4;

	Mem::Allocator& allocator;
	Slice<Type> items;
	usize sp;

	static Stack with_allocator(Mem::Allocator& allocator)
	{
		return Stack(allocator, 0, {});
	}

	static Stack with_size(Mem::Allocator& allocator, usize size)
	{
		return Stack(allocator, size, {});
	}

	Stack(Mem::Allocator& allocator, usize initial_size, const Slice<Type>& initial_content)
    : allocator(allocator)
    {
        usize initial_capacity = initial_size == 0 ? DefaultCapacity : initial_size;
        if(initial_size == 0)
        {
            initial_capacity = Math::min(DefaultCapacity, initial_content.len);
        }

        items = Mem::from_bytes<Type>(
            allocator.alloc(sizeof(Type) * initial_capacity, alignof(Type))
        );
        sp = 0;

        if(!initial_content.null())
        {
            Mem::copy(items, initial_content);
            sp = initial_content.len;
        }
    }

	~Stack()
	{
		if (items.ptr())
		{
			allocator.free(Mem::to_bytes(items));
			items = {};
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

		if (!allocator.realloc(Mem::to_bytes(items), sizeof(Type) * new_cap, alignof(Type)))
		{
			Slice new_items = allocator.array<Type>(new_cap);
			if (items.ptr())
			{
				Mem::copy(new_items, items);
				allocator.free(Mem::to_bytes(items));
			}

			items = new_items;
		}
		else
		{
			items.len = new_cap;
			Slice items_to_construct = items.add(sp);
			Mem::zero(items_to_construct);
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


