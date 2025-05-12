#include "platform/win32/win32_os.h"


static inline void _thread_handler(void* thread_data)
{
    Win32OS::ThreadData* data = (Win32OS::ThreadData*)thread_data;

    data->state = Win32OS::THREAD_STATE_RUNNING;
    data->fn(data->arg);
    data->state = Win32OS::THREAD_STATE_TERMINATED;

    ExitThread(0);
}

OS::VTable Win32OS::get_vtable()
{
    return OS::VTable
    {
        .initialize = &Win32OS::initialize,
        .shutdown = &Win32OS::shutdown,

        .get_page_size = &Win32OS::get_page_size,

        .thread_create = &Win32OS::thread_create,
        .thread_destroy = &Win32OS::thread_destroy,
        .thread_join = &Win32OS::thread_join,

        .mutex_create = &Win32OS::mutex_create,
        .mutex_destroy = &Win32OS::mutex_destroy,
        .mutex_lock = &Win32OS::mutex_lock,
        .mutex_try_lock = &Win32OS::mutex_try_lock,
        .mutex_unlock = &Win32OS::mutex_unlock,
    };
}

void Win32OS::initialize()
{
    ::new(data.threads_data) ThreadData[]{};
    ::new(data.mutex_data) MutexData[]{};

    // First data thread is reserved for main thread
    data.threads_data[0] = {};
    data.threads_data[0].handle = GetCurrentThread();
    data.threads_data->state = THREAD_STATE_RUNNING;
}

void Win32OS::shutdown()
{}

usize Win32OS::get_page_size()
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return (usize)info.dwPageSize;
}

OS::ThreadID Win32OS::thread_create(ThreadFn fn, void* arg)
{
    ThreadID tid = Win32OS::thread_data_allocate();
    ThreadData& thread_data = Win32OS::thread_data_get(tid);

    HANDLE thread_handle = CreateThread(
        nullptr, 0, (LPTHREAD_START_ROUTINE)_thread_handler, &thread_data,
        CREATE_SUSPENDED, 0
    );
    DebugAssert(thread_handle != 0, "can't create a new thread");

    thread_data.arg = arg;
    thread_data.fn = fn;
    thread_data.handle = thread_handle;
    thread_data.state = Win32OS::THREAD_STATE_NONE;

    ResumeThread(thread_data.handle);

    return tid;
}

void Win32OS::thread_destroy(ThreadID tid)
{
    DebugAssert(tid != ThreadID::InvalidID && tid < MaxThreadCount, "invalid thread id");
    FailOn(thread_join(tid) == false, "couldn't join the thread %d", tid);
    Win32OS::ThreadData& thread_data = Win32OS::thread_data_get(tid);
    CloseHandle((HANDLE)thread_data.handle);

    thread_data = {};
}

bool Win32OS::thread_join(ThreadID tid)
{
    DebugAssert(tid != ThreadID::InvalidID && tid < MaxThreadCount, "invalid thread id");
    Win32OS::ThreadData& thread_data = Win32OS::thread_data_get(tid);
    if (WaitForSingleObjectEx(thread_data.handle, INFINITE, FALSE) == WAIT_FAILED)
    {
        return false;
    }

    return true;
}

OS::MutexID Win32OS::mutex_create()
{
    MutexID mid = mutex_data_allocate();
    MutexData& mutex_data = mutex_data_get(mid);

    mutex_data.srw = SRWLOCK_INIT;

    return mid;
}

void Win32OS::mutex_destroy(MutexID mid)
{
    DebugAssert(mid != MutexID::InvalidID && mid < MaxMutexCount, "invalid thread id");
    MutexData& mutex_data = mutex_data_get(mid);
    mutex_data.allocated = false;
}

void Win32OS::mutex_lock(MutexID mid)
{
    DebugAssert(mid != MutexID::InvalidID && mid < MaxMutexCount, "invalid thread id");
    MutexData& mutex_data = mutex_data_get(mid);
    AcquireSRWLockExclusive(&mutex_data.srw);
}

bool Win32OS::mutex_try_lock(MutexID mid)
{
    DebugAssert(mid != MutexID::InvalidID && mid < MaxMutexCount, "invalid thread id");
    MutexData& mutex_data = mutex_data_get(mid);
    return TryAcquireSRWLockExclusive(&mutex_data.srw);
}

void Win32OS::mutex_unlock(MutexID mid)
{
    DebugAssert(mid != MutexID::InvalidID && mid < MaxMutexCount, "invalid thread id");
    MutexData& mutex_data = mutex_data_get(mid);
    ReleaseSRWLockExclusive(&mutex_data.srw);
}

OS::ThreadID Win32OS::thread_data_allocate()
{
    usize id = 1;

    for (; id < MaxThreadCount; id++)
    {
        if (data.threads_data[id].fn == nullptr)
        {
            return id;
        }
    }

    return ThreadID(id);
}

Win32OS::ThreadData& Win32OS::thread_data_get(ThreadID tid)
{
    DebugAssert(tid != ThreadID::InvalidID && tid < MaxThreadCount, "invalid thread id");
    return Win32OS::data.threads_data[tid];
}

OS::MutexID Win32OS::mutex_data_allocate()
{
    usize id = 0;

    for (; id < MaxMutexCount; id++)
    {
        if (data.mutex_data[id].allocated == false)
        {
            data.mutex_data[id].allocated = true;
            return id;
        }
    }

    return MutexID(id);
}

Win32OS::MutexData& Win32OS::mutex_data_get(MutexID mid)
{
    DebugAssert(mid != MutexID::InvalidID && mid < MaxMutexCount, "invalid thread id");
    return Win32OS::data.mutex_data[mid];
}
