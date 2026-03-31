#pragma once
#include "core/header.h"
#include "collections/string.h"


struct OS
{
    enum MapAccess
    {
        MapUnknown = 0,
        MapReadWrite,
        MapReadWriteExecute,
    };

    using ThreadID = ID<u32, struct __ThreadTag>;
    using ThreadFn = void(*)(Opaque*);
    using MutexID = ID<u32, struct __MutexTag>;
    using SemaphoreID = ID<u32, struct __SemaphoreTag>;

    using VoidFunction = void(*)();

    struct QueryMemory
    {
        MemoryAddress base_address;
        usize region_size;
    };

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

    static f64 get_time();

    static void exit(u64 code);
    static usize get_page_size();

    static MemoryAddress load_library(StringView lib_path);
    static void unload_library(MemoryAddress library);
    static VoidFunction get_proc_address(MemoryAddress library, StringView symbol_name);

    static Slice<u8> map_memory(usize memory_size, MapAccess access);
    static void unmap_memory(Slice<u8> memory);
    static QueryMemory query_memory(Slice<u8> memory);

    static ThreadID thread_create(ThreadFn fn, Opaque* arg);
    static void thread_destroy(ThreadID tid);
    static bool thread_join(ThreadID tid);
    static void thread_set_name(ThreadID tid, StringView new_name);
    static StringView thread_get_name(ThreadID tid);

    static MutexID mutex_create();
    static void mutex_destroy(MutexID mid);
    static void mutex_lock(MutexID mid);
    static bool mutex_try_lock(MutexID mid);
    static void mutex_unlock(MutexID mid);

    static SemaphoreID semaphore_create(usize initial_value);
    static void semaphore_destroy(SemaphoreID sid);
    static void semaphore_signal(SemaphoreID sid);
    static void semaphore_wait(SemaphoreID sid);

    static bool set_current_directory(StringView dir);
};
