#pragma once
#include "core/templates.h"


template<typename T>
struct Singleton
{
	static auto& get()
	{
		static inline T instance{};
		return instance;
	}
};