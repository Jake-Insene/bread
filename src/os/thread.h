#pragma once
#include "os/os.h"

struct [[nodiscard]] Thread
{
	OS::ThreadID id;

	static Thread create(OS::ThreadFn fn, Opaque* arg);

	void destroy() const;

	[[nodiscard]] bool join() const;
};
