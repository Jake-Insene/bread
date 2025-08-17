#include "platform/win32/win32_os.h"


static inline void _mutex_lock(SRWLOCK* srw)
{
    AcquireSRWLockExclusive(srw);
}

static inline bool _mutex_try_lock(SRWLOCK* srw)
{
    return TryAcquireSRWLockExclusive(srw);
}

static inline void _mutex_unlock(SRWLOCK* srw)
{
    ReleaseSRWLockExclusive(srw);
}


static inline UINT _thread_handler(void* thread_data)
{
    Win32OS::ThreadData* data = (Win32OS::ThreadData*)thread_data;

    _mutex_lock(&data->thread_srw);
    data->state = Win32OS::THREAD_STATE_RUNNING;
    _mutex_unlock(&data->thread_srw);

    data->fn(data->arg);

    _mutex_lock(&data->thread_srw);
    data->state = Win32OS::THREAD_STATE_TERMINATED;
    _mutex_unlock(&data->thread_srw);

    ExitThread(0);
    return 0;
}

void OS::initialize()
{
    ::new(Win32OS::data.threads_data) Win32OS::ThreadData[]{};
    ::new(Win32OS::data.mutex_data) Win32OS::MutexData[]{};

    // First data thread is reserved for main thread
    Win32OS::data.threads_data[0] = {};
    Win32OS::data.threads_data[0].handle = GetCurrentThread();
    Win32OS::data.threads_data->state = Win32OS::THREAD_STATE_RUNNING;
}

void OS::shutdown()
{}

void OS::exit(u64 code)
{
    ExitProcess((UINT)code);
}

usize OS::get_page_size()
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return (usize)info.dwPageSize;
}

OS::ThreadID OS::thread_create(OS::ThreadFn fn, void* arg)
{
    ThreadID tid = Win32OS::thread_data_allocate();
    Win32OS::ThreadData& thread_data = Win32OS::thread_data_get(tid);

    HANDLE thread_handle = CreateThread(
        nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(_thread_handler), &thread_data,
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

void OS::thread_destroy(ThreadID tid)
{
    DebugAssert(tid != ThreadID::InvalidID && tid < Win32OS::MaxThreadCount, "invalid thread id");
    FailOn(thread_join(tid) == false, "couldn't join the thread {}", tid.id);
    Win32OS::ThreadData& thread_data = Win32OS::thread_data_get(tid);
    CloseHandle((HANDLE)thread_data.handle);

    thread_data = {};
}

bool OS::thread_join(ThreadID tid)
{
    DebugAssert(tid != ThreadID::InvalidID && tid < Win32OS::MaxThreadCount, "invalid thread id");
    Win32OS::ThreadData& thread_data = Win32OS::thread_data_get(tid);
    if (WaitForSingleObjectEx(thread_data.handle, INFINITE, FALSE) == WAIT_FAILED)
    {
        return false;
    }

    return true;
}

OS::MutexID OS::mutex_create()
{
    MutexID mid = Win32OS::mutex_data_allocate();
    Win32OS::MutexData& mutex_data = Win32OS::mutex_data_get(mid);

    mutex_data.srw = SRWLOCK_INIT;

    return mid;
}

void OS::mutex_destroy(MutexID mid)
{
    DebugAssert(mid != MutexID::InvalidID && mid < Win32OS::MaxMutexCount, "invalid thread id");
    Win32OS::MutexData& mutex_data = Win32OS::mutex_data_get(mid);
    mutex_data.allocated = false;
}

void OS::mutex_lock(MutexID mid)
{
    DebugAssert(mid != MutexID::InvalidID && mid < Win32OS::MaxMutexCount, "invalid thread id");
    Win32OS::MutexData& mutex_data = Win32OS::mutex_data_get(mid);
    _mutex_lock(&mutex_data.srw);
}

bool OS::mutex_try_lock(MutexID mid)
{
    DebugAssert(mid != MutexID::InvalidID && mid < Win32OS::MaxMutexCount, "invalid thread id");
    Win32OS::MutexData& mutex_data = Win32OS::mutex_data_get(mid);
    return _mutex_try_lock(&mutex_data.srw);
}

void OS::mutex_unlock(MutexID mid)
{
    DebugAssert(mid != MutexID::InvalidID && mid < Win32OS::MaxMutexCount, "invalid thread id");
    Win32OS::MutexData& mutex_data = Win32OS::mutex_data_get(mid);
    _mutex_unlock(&mutex_data.srw);
}

bool OS::set_current_directory(StringView dir)
{
    char path[256]{};
    mem::copy(Slice(path), dir);
    if (SetCurrentDirectory(dir.ptr()))
        return true;
    return false;
}

OS::ThreadID Win32OS::thread_data_allocate()
{
    usize id = 1;

    for (; id < Win32OS::MaxThreadCount; id++)
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
