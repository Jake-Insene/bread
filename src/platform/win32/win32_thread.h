#pragma once
#include "os/thread.h"
#include "platform/platform_header.h"


struct Win32Thread
{
    enum class ThreadState
    {
        Unknown = 0,
        Running,
        Terminated,
    };


    HANDLE thread;
    SRWLOCK srw;

    ThreadState state;
    Thread::ThreadFn fn;
    Opaque* arg;
};
