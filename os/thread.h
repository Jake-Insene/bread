#pragma once
#include "os/os.h"

struct [[nodiscard]] Thread
{
	OS::ThreadID id;

	static Thread create(OS::ThreadFn fn, void* arg);

	void destroy() const;

	[[nnodiscard]] bool join() const;
};


struct [[nodiscard]] ScopedThread
{
	Thread thread;
	ScopedThread(OS::ThreadFn fn, void* arg)
		: thread(Thread::create(fn, arg))
	{}
	~ScopedThread()
	{
		thread.join();
	}
};
