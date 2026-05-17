#include "platform/win32/win32_thread.h"

#include "platform/win32/win32_os.h"


static inline Win32Thread* _get_thread_data(Opaque* impl)
{
    return impl->cast<Win32Thread*>();
}

static inline DWORD WINAPI _thread_handler(void* _arg)
{
    Win32Thread* data = _get_thread_data(reinterpret_cast<Opaque*>(_arg));

    AcquireSRWLockExclusive(&data->srw);
    data->state = Win32Thread::ThreadState::Running;
    ReleaseSRWLockExclusive(&data->srw);

    data->fn(data->arg);

    AcquireSRWLockExclusive(&data->srw);
    data->state = Win32Thread::ThreadState::Terminated;
    ReleaseSRWLockExclusive(&data->srw);

    ExitThread(0);
}

Thread Thread::create(ThreadFn fn, Opaque* arg)
{
    Thread thread = {};

    Win32Thread* data = Win32OS::get_allocator()->object<Win32Thread>();
    data->thread = nullptr;
    data->srw = SRWLOCK_INIT;
    data->state = Win32Thread::ThreadState::Unknown;
    data->fn = fn;
    data->arg = arg;

    data->thread = CreateThread(
        nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(&_thread_handler), data,
        CREATE_SUSPENDED, 0
    );

    ResumeThread(data->thread);
    
    thread.impl = Opaque::from(*data);
    return thread;
}

void Thread::destroy() const
{
    Win32Thread* data = _get_thread_data(impl);

    WaitForSingleObjectEx(data->thread, INFINITE, FALSE);
    CloseHandle(data->thread);

    Win32OS::get_allocator()->free(mem::to_bytes(Slice(data, 1)));
}

bool Thread::join() const
{
    Win32Thread* data = _get_thread_data(impl);
    return WaitForSingleObjectEx(data->thread, INFINITE, FALSE) != WAIT_FAILED;
}