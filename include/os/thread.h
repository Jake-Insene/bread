#pragma once
#include "core/header.h"



struct [[nodiscard]] Thread
{
    using ThreadFn = void(*)(Opaque*);
	
	Opaque* impl;

	static Thread create(ThreadFn fn, Opaque* arg);

	void destroy() const;

	[[nodiscard]] bool join() const;
};
