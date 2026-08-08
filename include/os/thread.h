#pragma once
#include "Core/Header.h"



struct [[nodiscard]] Thread
{
    using ThreadFn = void(*)(Core::Opaque*);
	
	Core::Opaque* impl;

	static Thread create(ThreadFn fn, Core::Opaque* arg);

	void destroy() const;

	[[nodiscard]] bool join() const;
};
