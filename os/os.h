#pragma once
#include "core/string.h"
#include "core/id.h"
#include "core/vtable.h"


struct OS
{
    using ThreadID = ID<u32>;
    using ThreadFn = void(*)(void*);

    using MutexID = ID<u32>;

    static void initialize();
    static void shutdown();

    static void exit(u64);
    static usize get_page_size();

    static ThreadID thread_create(ThreadFn, void*);
    static void thread_destroy(ThreadID);
    static bool thread_join(ThreadID);

    static MutexID mutex_create();
    static void mutex_destroy(ThreadID);
    static void mutex_lock(ThreadID);
    static bool mutex_try_lock(MutexID);
    static void mutex_unlock(MutexID);

    static bool set_current_directory(StringView dir);
};
