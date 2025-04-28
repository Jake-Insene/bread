#include "os/thread.h"

#include "platform/platform_header.h"


Thread Thread::create(ThreadFn fn, void* arg)
{
	usize handle = 0;
	handle = (usize)CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)fn, arg, 0, 0);

	return Thread(handle);
}

void Thread::destroy() const
{
	CloseHandle((HANDLE)handle);
}

bool Thread::join() const
{
	if (WaitForSingleObjectEx((HANDLE)handle, INFINITE, FALSE) == WAIT_FAILED)
	{
		return false;
	}

	return true;
}
