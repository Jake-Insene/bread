#pragma once
#include "collections/stack.h"
#include "os/mutex.h"


/* 
* TODO: Thread safe by default
*/
struct [[nodiscard]] JobQueue
{
	struct JobInfo
	{
		void(*func)(Opaque* arg);
		Opaque* arg;
	};

	Mem::Allocator& allocator;
	Stack<JobInfo> job_stack;
	Mutex mutex;

	static JobQueue with_allocator(Mem::Allocator& allocator);

	static JobQueue with_size(Mem::Allocator& allocator, usize initial_size);

	JobQueue(Mem::Allocator& allocator, usize initial_size);
	~JobQueue();

	template<typename Fn>
	void add_job(Fn&& _job)
	{
		mutex.lock();
		Fn* fn_mem = reinterpret_cast<Fn*>(
			allocator.alloc(sizeof(Fn), alignof(usize)).ptr()
		);
		ConstructObject(*fn_mem, _job);

		JobInfo job =
		{
			.func = [](Opaque* arg) { Invoke(*arg->cast<Fn*>()); },
			.arg = reinterpret_cast<Opaque*>(fn_mem),
		};

		job_stack.push(job);
		mutex.unlock();
	}

	void run();
};