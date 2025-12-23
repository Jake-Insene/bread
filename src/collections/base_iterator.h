#pragma once
#include "core/header.h"


/*
* Contains simple routines for iterators.
* 
* @param Iterator Must implement begin()/end()
*/
template<typename T>
struct [[nodiscard]] BaseIterator
{
	template<typename Self>
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

	template<typename Fn>
	constexpr auto for_each(this auto&& self, Fn&& fn)
	{
		using ItFnComplete1 = void(*)(T&, usize);
		using ItFnComplete2 = void(*)(const T&, usize);
		
		usize index = 0;
		for (auto&& item : self)
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

		return Forward<decltype(self)>(self);
	}

	template<typename Self, typename Fn>
	constexpr auto transform(this auto&& self, Fn&& op)
	{
		for (auto it = self.begin(); it != self.end(); ++it)
		{
			*it = op(*it);
		}

		return Forward<decltype(self)>(self);
	}

};