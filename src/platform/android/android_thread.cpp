#include "platform/android/android_thread.h"


Thread Thread::create(ThreadFn fn, Opaque* arg)
{
    Thread thread = {};
    return thread;
}

void Thread::destroy() const
{
}

bool Thread::join() const
{
    return false;
}
