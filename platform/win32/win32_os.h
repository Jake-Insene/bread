#pragma once
#include "os/os.h"

#include "os/thread.h"
#include "platform/platform_header.h"


struct Win32OS : OS
{
    static constexpr usize MaxThreadCount = 16;
    static constexpr usize MaxMutexCount = 1024;

    static constexpr usize MaxThreadNameLen = 128;

    enum ThreadState
    {
        THREAD_STATE_NONE = 0,
        THREAD_STATE_RUNNING = 1,
        THREAD_STATE_TERMINATED = 2,
    };

    struct ThreadData
    {
        void* arg;
        OS::ThreadFn fn;

        HANDLE handle;
        ThreadState state;
    };

    struct MutexData
    {
        SRWLOCK srw;
        ThreadID lock_owner;

        bool allocated;
    };

    struct InternalData
    {
        ThreadData threads_data[MaxThreadCount];
        MutexData mutex_data[MaxMutexCount];
    };

    static inline InternalData data;

    static OS::VTable get_vtable();

    static void initialize();
    static void shutdown();

    static usize get_page_size();

    static ThreadID thread_create(ThreadFn fn, void* arg);
    static void thread_destroy(ThreadID tid);
    static bool thread_join(ThreadID tid);

    static MutexID mutex_create();
    static void mutex_destroy(MutexID mid);
    static void mutex_lock(MutexID mid);
    static bool mutex_try_lock(MutexID mid);
    static void mutex_unlock(MutexID mid);

    static ThreadID thread_data_allocate();
    static ThreadData& thread_data_get(ThreadID tid);

    static MutexID mutex_data_allocate();
    static MutexData& mutex_data_get(MutexID mid);
};
