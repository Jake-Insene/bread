#include "os/thread.h"

#include "os/os.h"


Thread Thread::create(OS::ThreadFn fn, void* arg)
{
    return Thread
    {
        .id = OS::thread_create(fn, arg)
    };
}

void Thread::destroy() const
{
    OS::thread_destroy(id);
}

bool Thread::join() const
{
    return OS::thread_join(id);
}

