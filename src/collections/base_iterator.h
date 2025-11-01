#pragma once
#include "core/header.h"


/*
* Contains simple routines for iterators.
* 
* @param Iterator Must implement begin()/end()
*/
template<typename Iterator, typename T>
struct [[nodiscard]] BaseIterator
{
	Iterator find(const T& item_requested) const
	{
		const Iterator& iterable = static_cast<const Iterator&>(*this);

		for (auto it = iterable.begin(); it != iterable.end(); ++it)
		{
			if (*it == item_requested)
			{
				return it;
			}
		}

		return iterable.end();
	}

	template<typename Fn>
	void for_each(Fn&& fn)
	{
		using ItFnComplete1 = void(*)(T&, usize);
		using ItFnComplete2 = void(*)(const T&, usize);
		
		const Iterator& it = static_cast<const Iterator&>(*this);
		usize index = 0;
		for (auto&& item : it)
		{
			if constexpr (IsAnyOf<Fn, ItFnComplete1, ItFnComplete2>)
			{
				fn(item, index++);
			}
			else
			{
				fn(item);
			}
		}
	}

	template<typename Fn>
	Iterator& transform(Fn&& op)
	{
		Iterator& iterable = static_cast<Iterator&>(*this);

		for (auto it = iterable.begin(); it != iterable.end(); ++it)
		{
			*it = op(*it);
		}

		return iterable;
	}

};