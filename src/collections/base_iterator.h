#pragma once
#include "core/header.h"
#include "core/templates.h"



template<typename IteratorType>
concept Iterable = requires(IteratorType& it)
{
	{ it.begin() };
	{ it.end() };
};


/*
* Contains simple routines for iterators.
* 
* @param Iterator Must implement begin()/end()
*/
template<typename T>
struct [[nodiscard]] BaseIterator
{
	template<Iterable Self>
	constexpr auto find(this Self const& self, const T& item_requested)
	{
		for (auto it = self.begin(); it != self.end(); ++it)
		{
			if (*it == item_requested)
			{
				return it;
			}
		}

		return self.end();
	}

	template<Iterable Self, typename Fn>
	constexpr auto for_each(this Self&& self, Fn&& fn)
	{
		using ItFnComplete1 = void(*)(T&, usize);
		using ItFnComplete2 = void(*)(const T&, usize);
		
		usize index = 0;
		for (auto&& item : self)
		{
			if constexpr (IsAnyOf<Fn, ItFnComplete1, ItFnComplete2>)
			{
				Invoke(fn, item, index++);
			}
			else
			{
				Invoke(fn, item);
			}
		}

		return Forward<decltype(self)>(self);
	}

	template<Iterable Self, typename Fn>
	constexpr auto transform(this Self&& self, Fn&& op)
	{
		for (auto it = self.begin(); it != self.end(); ++it)
		{
			*it = op(*it);
		}

		return Forward<decltype(self)>(self);
	}

	template<Iterable Self, typename Fn>
	constexpr auto filter(this Self&& self, Fn&& op, Fn&& fn)
	{
		for (auto it = self.begin(); it != self.end(); ++it)
		{
			if(op(*it))
			{
				fn(*it);
			}
		}
		
		return Forward<decltype(self)>(self);
	}
};