#include "concurrency/job_queue.h"


JobQueue JobQueue::with_allocator(Mem::Allocator* allocator)
{
	return JobQueue
	{
		.allocator = allocator,
		.job_stack = Stack<JobInfo>::with_allocator(allocator),
		.mutex = Mutex::create(),
	};
}

JobQueue JobQueue::with_size(Mem::Allocator* allocator, usize initial_size)
{
	return JobQueue
	{
		.allocator = allocator,
		.job_stack = Stack<JobInfo>::with_size(allocator, initial_size),
		.mutex = Mutex::create(),
	};
}

void JobQueue::destroy()
{
	job_stack.destroy();
	mutex.destroy();
}

void JobQueue::run()
{
	mutex.lock();

	while (!job_stack.is_empty())
	{
		JobInfo job = job_stack.pop();
		job.func(job.arg);
		allocator->free(Slice(job.arg->cast<u8*>(), 1));
	}

	mutex.unlock();
}
