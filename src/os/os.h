#pragma once
#include "collections/string.h"
#include "core/id.h"
#include "core/vtable.h"


struct OS
{
    using ThreadID = ID<u32>;
    using ThreadFn = void(*)(void*);

    using MutexID = ID<u32>;

    using SemaphoreID = ID<u32>;

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

    static void exit(u64 code);
    static usize get_page_size();

    static ThreadID thread_create(ThreadFn fn, void* arg);
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
