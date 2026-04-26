#pragma once
#include "collections/free_list.h"
#include "os/os.h"
#include "platform/platform_header.h"



// TODO: Make multithread functions thread safe and multithread.
struct Win32OS
{
    static constexpr usize InitialThreadCount = 16;
    static constexpr usize InitialMutexCount = 1024;
    static constexpr usize InitialSemaphoreCount = 1024;

    static constexpr usize MaxThreadNameLen = 128;

    enum class ThreadState
    {
        Unknown = 0,
        Running,
        Terminated,
    };

    struct ThreadData
    {
        char name[MaxThreadNameLen];

        Opaque* arg;
        OS::ThreadFn fn;

        HANDLE handle;
        ThreadState state;

        SRWLOCK thread_srw;
    };

    struct MutexData
    {
        SRWLOCK srw;
        OS::ThreadID lock_owner;
    };

    struct SemaphoreData
    {
        HANDLE handle;
    };

    struct InternalData
    {
        mem::Allocator allocator;

        i64 frequency;
        f64 program_start;
        usize page_size;

        SRWLOCK thread_allocate_srw;
        FreeList<ThreadData, OS::ThreadID> threads;

        SRWLOCK mutex_allocate_srw;
        FreeList<MutexData, OS::MutexID> mutexes;
        
        SRWLOCK semaphore_allocate_srw;
        FreeList<SemaphoreData, OS::SemaphoreID> semaphores;
    };

    static inline Win32OS::InternalData data;

    [[nodiscard]] static mem::Allocator& get_allocator() { return data.allocator; }

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

    static f64 get_time();

    static void exit(u64 code);
    static usize get_page_size();

    static MemoryAddress load_library(StringView lib_path);
    static void unload_library(MemoryAddress library);
    static OS::VoidFunction get_proc_address(MemoryAddress library, StringView symbol_name);

    static Slice<u8> map_memory(usize memory_size, OS::MapAccess access);
    static void unmap_memory(const Slice<u8>& memory);
    static OS::QueryMemory query_memory(const Slice<u8>& memory);

    static OS::ThreadID thread_create(OS::ThreadFn fn, Opaque* arg);
    static void thread_destroy(OS::ThreadID tid);
    static bool thread_join(OS::ThreadID tid);
    static void thread_set_name(OS::ThreadID tid, StringView new_name);
    static StringView thread_get_name(OS::ThreadID tid);

    static OS::MutexID mutex_create();
    static void mutex_destroy(OS::MutexID mid);
    static void mutex_lock(OS::MutexID mid);
    static bool mutex_try_lock(OS::MutexID mid);
    static void mutex_unlock(OS::MutexID mid);

    static OS::SemaphoreID semaphore_create(usize initial_value);
    static void semaphore_destroy(OS::SemaphoreID sid);
    static void semaphore_signal(OS::SemaphoreID sid);
    static void semaphore_wait(OS::SemaphoreID sid);

    static bool set_current_directory(StringView dir);

    static OS::ThreadID _thread_data_allocate();
    static ThreadData& _thread_data_get(OS::ThreadID tid);

    static OS::MutexID _mutex_data_allocate();
    static MutexData& _mutex_data_get(OS::MutexID mid);

    static OS::SemaphoreID _semaphore_data_allocate();
    static SemaphoreData& _semaphore_data_get(OS::SemaphoreID sid);
};
