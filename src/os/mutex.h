#pragma once
#include "core/header.h"


struct [[nodiscard]] Mutex
{
    Opaque* impl;

    static Mutex create();

    void destroy();

    void lock();
    bool try_lock();
    void unlock();
};

