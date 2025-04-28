#pragma once
#include "core/types.h"


struct [[nodiscard]] Thread
{
	usize handle;

	using ThreadFn = void(*)(void*);

	static Thread create(ThreadFn fn, void* arg);

	void destroy() const;

	bool join() const;
};


struct [[nodiscard]] ScopedThread
{
	Thread thread;
	ScopedThread(Thread::ThreadFn fn, void* arg)
		: thread(Thread::create(fn, arg))
	{
	}
	~ScopedThread()
	{
		thread.join();
	}
};
