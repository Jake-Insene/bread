#pragma once
#include "core/types.h"
#include "core/id.h"
#include "core/vtable.h"


struct OS
{
    using ThreadID = ID<u32>;
    using ThreadFn = void(*)(void*);

    using MutexID = ID<u32>;

    struct VTable
    {
        VTFunc(void, initialize);
        VTFunc(void, shutdown);

        VTFunc(void, exit, u64);
        VTFunc(usize, get_page_size);

        VTFunc(ThreadID, thread_create, ThreadFn, void*);
        VTFunc(void, thread_destroy, ThreadID);
        VTFunc(bool, thread_join, ThreadID);

        VTFunc(MutexID, mutex_create);
        VTFunc(void, mutex_destroy, MutexID);
        VTFunc(void, mutex_lock, MutexID);
        VTFunc(bool, mutex_try_lock, MutexID);
        VTFunc(void, mutex_unlock, MutexID);
    };

    static inline VTable vtable;

    static void initialize();

    VTFuncDefS(shutdown);

    VTFuncDefArg1S(exit, u64);
    VTFuncDefRetS(usize, get_page_size);

    VTFuncDefArg2RetS(ThreadID, thread_create, ThreadFn, void*);
    VTFuncDefArg1S(thread_destroy, ThreadID);
    VTFuncDefArg1RetS(bool, thread_join, ThreadID);

    VTFuncDefRetS(MutexID, mutex_create);
    VTFuncDefArg1S(mutex_destroy, ThreadID);
    VTFuncDefArg1S(mutex_lock, ThreadID);
    VTFuncDefArg1RetS(bool, mutex_try_lock, MutexID);
    VTFuncDefArg1S(mutex_unlock, MutexID);
};
