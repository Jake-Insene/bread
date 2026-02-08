#pragma once
#include "core/templates.h"


template<typename T>
struct Singleton
{
	static auto& get()
	{
		static T instance = {};
		return instance;
	}
};