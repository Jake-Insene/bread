#include "concurrency/job_queue.h"


JobQueue::JobQueue(Mem::Allocator* allocator, usize initial_size)
: allocator(allocator), job_stack(allocator, initial_size, {}),
mutex(Mutex::create())
{}

JobQueue::~JobQueue()
{
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
