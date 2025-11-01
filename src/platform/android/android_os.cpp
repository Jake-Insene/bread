#include "platform/android/android_os.h"

#include "core/header.h"
#include "platform/platform_header.h"


void AndroidOS::initialize(const mem::Allocator& allocator)
{
    data.threads = FreeList<ThreadData,OS::ThreadID>::with_size(allocator, InitialThreadCount);
    data.mutexes = FreeList<MutexData, OS::MutexID>::with_size(allocator, InitialMutexCount);

    // First data thread is reserved for main thread
    OS::ThreadID main_thread = thread_data_allocate();
    auto& thread_data = thread_data_get(main_thread);

    thread_data.state = THREAD_STATE_RUNNING;
}

void AndroidOS::shutdown()
{
    data.mutexes.destroy();
    data.threads.destroy();
}

f64 AndroidOS::get_time()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

void AndroidOS::exit(u64 code)
{}

usize AndroidOS::get_page_size()
{
    return (usize)sysconf(_SC_PAGESIZE);
}

Slice<u8> AndroidOS::map_memory(usize memory_size, OS::MapAccess access)
{
    const usize aligned_size = mem::align_up(memory_size, get_page_size());
    Slice<u8> ptr{};

    switch (access)
    {
    case OS::MapUnknown:
        break;
    case OS::MapReadWrtie:
    {
        ptr.items = (u8*)mmap(
            0, aligned_size,
            PROT_READ | PROT_WRITE,
            MAP_ANONYMOUS | MAP_PRIVATE,
            -1, 0
        );
        ptr.len = aligned_size;
    }
    break;
    }

    return ptr;
}

void AndroidOS::unmap_memory(Slice<u8> memory)
{
    munmap(memory.items, memory.len);
}

OS::ThreadID AndroidOS::thread_create(OS::ThreadFn fn, Opaque* arg)
{
    return OS::ThreadID();
}

void AndroidOS::thread_destroy(OS::ThreadID tid)
{
}

bool AndroidOS::thread_join(OS::ThreadID tid)
{
    return false;
}

void AndroidOS::thread_set_name(OS::ThreadID tid, StringView new_name)
{
}

StringView AndroidOS::thread_get_name(OS::ThreadID tid)
{
    return StringView();
}

OS::MutexID AndroidOS::mutex_create()
{
    return OS::MutexID();
}

void AndroidOS::mutex_destroy(OS::MutexID mid)
{
}

void AndroidOS::mutex_lock(OS::MutexID mid)
{
}

bool AndroidOS::mutex_try_lock(OS::MutexID mid)
{
    return false;
}

void AndroidOS::mutex_unlock(OS::MutexID mid)
{
}


OS::SemaphoreID AndroidOS::semaphore_create(usize initial_value)
{
    return OS::SemaphoreID();
}

void AndroidOS::semaphore_destroy(OS::SemaphoreID sid)
{
}

void AndroidOS::semaphore_signal(OS::SemaphoreID sid)
{
}

void AndroidOS::semaphore_wait(OS::SemaphoreID sid)
{
}

bool AndroidOS::set_current_directory(StringView dir)
{
    return true;
}

OS::ThreadID AndroidOS::thread_data_allocate()
{
    return data.threads.add(ThreadData());
}

AndroidOS::ThreadData& AndroidOS::thread_data_get(OS::ThreadID tid)
{
    return data.threads.get(tid);
}

OS::MutexID AndroidOS::mutex_data_allocate()
{
    return data.mutexes.add(MutexData());
}

AndroidOS::MutexData& AndroidOS::mutex_data_get(OS::MutexID mid)
{
    return data.mutexes.get(mid);
}

OS::SemaphoreID AndroidOS::semaphore_data_allocate()
{
    return data.semaphores.add(SemaphoreData());
}

AndroidOS::SemaphoreData& AndroidOS::semaphore_data_get(OS::SemaphoreID sid)
{
    return data.semaphores.get(sid);
}
