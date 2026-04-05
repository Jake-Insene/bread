#include "platform/win32/win32_os.h"

#include "core/types.h"
#include "debug/fail.h"


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

static inline DWORD WINAPI _thread_handler(void* _arg)
{
    Opaque* thread_data = reinterpret_cast<Opaque*>(_arg);
    Win32OS::ThreadData* data = thread_data->cast<Win32OS::ThreadData*>();

    _mutex_lock(&data->thread_srw);
    data->state = Win32OS::THREAD_STATE_RUNNING;
    _mutex_unlock(&data->thread_srw);

    data->fn(data->arg);

    _mutex_lock(&data->thread_srw);
    data->state = Win32OS::THREAD_STATE_TERMINATED;
    _mutex_unlock(&data->thread_srw);

    ExitThread(0);
}

void Win32OS::initialize(const mem::Allocator& allocator)
{
    data.allocator = allocator;
    
    // For get_time()
    LARGE_INTEGER platform_time;
    QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&data.frequency));
    QueryPerformanceCounter(&platform_time);

    data.program_start = f64(platform_time.QuadPart) / f64(data.frequency);

    SYSTEM_INFO info;
    GetSystemInfo(&info);
    data.page_size = static_cast<usize>(info.dwPageSize);

    data.threads = FreeList<ThreadData,OS::ThreadID>::with_size(allocator, InitialThreadCount);
    data.mutexes = FreeList<MutexData, OS::MutexID>::with_size(allocator, InitialMutexCount);
    data.semaphores = FreeList<SemaphoreData, OS::SemaphoreID>::with_size(allocator, InitialSemaphoreCount);

    // First data thread is reserved for main thread
    OS::ThreadID main_thread = _thread_data_allocate();
    auto& thread_data = _thread_data_get(main_thread);

    thread_data.handle = GetCurrentThread();
    thread_data.state = THREAD_STATE_RUNNING;
    thread_data.thread_srw = SRWLOCK_INIT;
}

void Win32OS::shutdown()
{
    data.mutexes.destroy();
    data.threads.destroy();
    data.semaphores.destroy();
}

f64 Win32OS::get_time()
{
    LARGE_INTEGER platform_time;
    QueryPerformanceCounter(&platform_time);
    return (f64(platform_time.QuadPart) / f64(data.frequency)) - data.program_start;
}

void Win32OS::exit(u64 code)
{
    ExitProcess(static_cast<UINT>(code));
}

usize Win32OS::get_page_size()
{
    return data.page_size;
}

MemoryAddress Win32OS::load_library(StringView lib_path)
{
    return reinterpret_cast<MemoryAddress>(LoadLibraryA(lib_path.ptr()));
}

void Win32OS::unload_library(MemoryAddress library)
{
    FreeLibrary(reinterpret_cast<HMODULE>(library));
}

OS::VoidFunction Win32OS::get_proc_address(MemoryAddress library, StringView symbol_name)
{
    Slice<char> chars = get_allocator().array<char>(symbol_name.len + 1);
    mem::copy(chars, symbol_name);
    chars[chars.len-1] = 0;

    OS::VoidFunction func = reinterpret_cast<OS::VoidFunction>(
        GetProcAddress(reinterpret_cast<HMODULE>(library), chars.ptr())
    );

    get_allocator().free(mem::to_bytes(chars));
    return func;
}

Slice<u8> Win32OS::map_memory(usize memory_size, OS::MapAccess access)
{
    const usize aligned_size = mem::align_up(memory_size, get_page_size());
    Slice<u8> ptr{};

    switch (access)
    {
    case OS::MapUnknown:
        break;
    case OS::MapReadWrite:
    {
        ptr.items = reinterpret_cast<u8*>(
            VirtualAllocEx(GetCurrentProcess(),
                nullptr, aligned_size,
                MEM_RESERVE | MEM_COMMIT,
                PAGE_READWRITE
            )
        );
        ptr.len = aligned_size;
    }
        break;
    case OS::MapReadWriteExecute:
    {
        ptr.items = reinterpret_cast<u8*>(
            VirtualAllocEx(GetCurrentProcess(),
                nullptr, aligned_size,
                MEM_RESERVE | MEM_COMMIT,
                PAGE_EXECUTE_READWRITE
            )
        );
        ptr.len = aligned_size;
    }
        break;
    }

    return ptr;
}

void Win32OS::unmap_memory(Slice<u8> memory)
{
    VirtualFreeEx(GetCurrentProcess(), memory.items, 0, MEM_RELEASE);
}

OS::QueryMemory Win32OS::query_memory(Slice<u8> memory)
{
    MEMORY_BASIC_INFORMATION mem_info;
    VirtualQueryEx(GetCurrentProcess(), memory.items, &mem_info, sizeof(mem_info));
    
    return OS::QueryMemory
    {
        .base_address = MemoryAddress(mem_info.BaseAddress),
        .region_size = usize(mem_info.RegionSize),
    };    
}

OS::ThreadID Win32OS::thread_create(OS::ThreadFn fn, Opaque* arg)
{
    OS::ThreadID tid = _thread_data_allocate();
    ThreadData& thread_data = _thread_data_get(tid);

    HANDLE thread_handle = CreateThread(
        nullptr, 0, &_thread_handler, &thread_data,
        CREATE_SUSPENDED, 0
    );
    DebugAssert(thread_handle != nullptr, "can't create a new thread");
   
    mem::set(Slice(thread_data.name), 0i8);

    thread_data.arg = arg;
    thread_data.fn = fn;
    thread_data.handle = thread_handle;
    thread_data.state = Win32OS::THREAD_STATE_NONE;
    thread_data.thread_srw = SRWLOCK_INIT;

    ResumeThread(thread_data.handle);

    return tid;
}

void Win32OS::thread_destroy(OS::ThreadID tid)
{
    FailOn(thread_join(tid) == false, "couldn't join the thread {}", tid.id);

    ThreadData& thread_data = _thread_data_get(tid);
    CloseHandle(thread_data.handle);
 
    data.threads.remove(tid);
}

bool Win32OS::thread_join(OS::ThreadID tid)
{
    ThreadData& thread_data = _thread_data_get(tid);
    if (WaitForSingleObjectEx(thread_data.handle, INFINITE, FALSE) == WAIT_FAILED)
    {
        return false;
    }

    return true;
}

void Win32OS::thread_set_name(OS::ThreadID tid, StringView new_name)
{
    ThreadData& thread_data = _thread_data_get(tid);
    mem::copy(Slice(thread_data.name), new_name);

    SetThreadDescription(thread_data.handle, reinterpret_cast<PCWSTR>(thread_data.name));
}

StringView Win32OS::thread_get_name(OS::ThreadID tid)
{
    ThreadData& thread_data = _thread_data_get(tid);

    CHAR* name_address = nullptr;
    GetThreadDescription(thread_data.handle, reinterpret_cast<PWSTR*>(&name_address));
    usize len = __string_len(name_address);
    mem::copy(Slice(thread_data.name), Slice(name_address, len));

    return StringView(thread_data.name, len);
}

OS::MutexID Win32OS::mutex_create()
{
    OS::MutexID mid = _mutex_data_allocate();
    MutexData& mutex_data = _mutex_data_get(mid);

    mutex_data.srw = SRWLOCK_INIT;

    return mid;
}

void Win32OS::mutex_destroy(OS::MutexID mid)
{
    data.mutexes.remove(mid);
}

void Win32OS::mutex_lock(OS::MutexID mid)
{
    MutexData& mutex_data = _mutex_data_get(mid);
    _mutex_lock(&mutex_data.srw);
}

bool Win32OS::mutex_try_lock(OS::MutexID mid)
{
    MutexData& mutex_data = _mutex_data_get(mid);
    return _mutex_try_lock(&mutex_data.srw);
}

void Win32OS::mutex_unlock(OS::MutexID mid)
{
    MutexData& mutex_data = _mutex_data_get(mid);
    _mutex_unlock(&mutex_data.srw);
}


OS::SemaphoreID Win32OS::semaphore_create(usize initial_value)
{
    OS::SemaphoreID sid = _semaphore_data_allocate();
    SemaphoreData& semaphore_data = _semaphore_data_get(sid);
    
    semaphore_data.handle = CreateSemaphoreA(nullptr, LONG(initial_value), MaxValue<i32>, nullptr);
    DebugAssert(semaphore_data.handle != nullptr, "can't create a new semaphore");

    return sid;
}

void Win32OS::semaphore_destroy(OS::SemaphoreID sid)
{
    SemaphoreData& semaphore_data = _semaphore_data_get(sid);
    CloseHandle(semaphore_data.handle);
}

void Win32OS::semaphore_signal(OS::SemaphoreID sid)
{
    SemaphoreData& semaphore_data = _semaphore_data_get(sid);
    ReleaseSemaphore(semaphore_data.handle, 1, nullptr);
}

void Win32OS::semaphore_wait(OS::SemaphoreID sid)
{
    SemaphoreData& semaphore_data = _semaphore_data_get(sid);
    WaitForSingleObject(semaphore_data.handle, INFINITE);
}

bool Win32OS::set_current_directory(StringView dir)
{
    char path[256]{};
    mem::copy(Slice(path), dir);
    if (SetCurrentDirectory(dir.ptr()))
        return true;
    return false;
}

OS::ThreadID Win32OS::_thread_data_allocate()
{
    return data.threads.add(ThreadData());
}

Win32OS::ThreadData& Win32OS::_thread_data_get(OS::ThreadID tid)
{
    return data.threads.get(tid);
}

OS::MutexID Win32OS::_mutex_data_allocate()
{
    return data.mutexes.add(MutexData());
}

Win32OS::MutexData& Win32OS::_mutex_data_get(OS::MutexID mid)
{
    return data.mutexes.get(mid);
}

OS::SemaphoreID Win32OS::_semaphore_data_allocate()
{
    return data.semaphores.add(SemaphoreData());
}

Win32OS::SemaphoreData& Win32OS::_semaphore_data_get(OS::SemaphoreID sid)
{
    return data.semaphores.get(sid);
}
