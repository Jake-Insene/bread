#pragma once
#include "os/os.h"
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

        SRWLOCK thread_srw;
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

    static ThreadID thread_data_allocate();
    static ThreadData& thread_data_get(ThreadID tid);

    static MutexID mutex_data_allocate();
    static MutexData& mutex_data_get(MutexID mid);
};
