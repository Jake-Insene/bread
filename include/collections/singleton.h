#pragma once
#include "core/Templates.h"


template<typename T>
struct Singleton
{
	static auto& get()
	{
		static T instance = {};
		return instance;
	}
};