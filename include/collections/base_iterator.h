#pragma once
#include "Core/Header.h"



/*
* Contains simple routines for iterators.
* 
* @param Iterator Must implement begin()/end()
*/
template<typename T>
struct [[nodiscard]] BaseIterator
{
	using Type = T;

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
	constexpr auto for_each(this Self&& self, Fn&& func)
	{
		using ItFnComplete1 = void(*)(Type&, usize);
		using ItFnComplete2 = void(*)(const Type&, usize);
		
		usize index = 0;
		for (auto&& item : self)
		{
			if constexpr (Core::IsAnyOf<Fn, ItFnComplete1, ItFnComplete2>)
			{
				Core::Invoke(func, item, index++);
			}
			else
			{
				Core::Invoke(func, item);
			}
		}

		return Core::Forward<decltype(self)>(self);
	}

	template<Iterable Self, typename Fn>
	requires(Core::Returns<Type, Fn>)
	constexpr auto transform(this Self&& self, Fn&& opt)
	{
		for (auto it = self.begin(); it != self.end(); ++it)
		{
			*it = opt(*it);
		}

		return Core::Forward<decltype(self)>(self);
	}

	template<Iterable Self, typename Opt, typename Fn>
	requires(Core::Returns<bool, Opt>)
	constexpr auto filter(this Self&& self, Opt&& opt, Fn&& func)
	{
		for (auto it = self.begin(); it != self.end(); ++it)
		{
			if(opt(*it))
			{
				func(*it);
			}
		}
		
		return Forward<decltype(self)>(self);
	}
};