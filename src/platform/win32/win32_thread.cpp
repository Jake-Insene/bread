#include "platform/win32/win32_thread.h"



Thread Thread::create(ThreadFn fn, Opaque* arg)
{
    Thread thread = {};

    thread.impl = reinterpret_cast<Opaque*>(
        CreateThread(
            nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(&fn), arg,
            CREATE_SUSPENDED, 0
        )
    );

    return thread;
}

void Thread::destroy() const
{
    WaitForSingleObjectEx(reinterpret_cast<HANDLE>(impl), INFINITE, FALSE);
    CloseHandle(reinterpret_cast<HANDLE>(impl));
}

bool Thread::join() const
{
    return WaitForSingleObjectEx(reinterpret_cast<HANDLE>(impl), INFINITE, FALSE) != WAIT_FAILED;
}