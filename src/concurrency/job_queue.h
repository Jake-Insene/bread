#pragma once
#include "collections/stack.h"
#include "os/mutex.h"


/* 
* TODO:
* Thread safe by default
*/
struct [[nodiscard]] JobQueue
{
	struct JobInfo
	{
		void(*func)(Opaque* arg);
		Opaque* arg;
	};

	mem::Allocator allocator;
	Stack<JobInfo> job_stack;
	Mutex mutex;

	static JobQueue with_allocator(const mem::Allocator& allocator);

	static JobQueue with_size(const mem::Allocator& allocator, usize initial_size);

	void destroy();

	template<typename Fn>
	void add_job(Fn fn)
	{
		Fn* fn_mem = reinterpret_cast<Fn*>(
			allocator.alloc(sizeof(Fn), alignof(usize)).ptr()
		);
		ConstructObject(*fn_mem, fn);

		JobInfo job =
		{
			.func = [](Opaque* arg) { Invoke(*arg->cast<Fn*>()); },
			.arg = reinterpret_cast<Opaque*>(fn_mem),
		};

		mutex.lock();
		job_stack.push(job);
		mutex.unlock();
	}

	void run();
};