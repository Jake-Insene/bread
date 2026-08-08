#include "platform/win32/win32_semaphore.h"

#include "debug/assertion.h"


Semaphore Semaphore::create(usize initial_value)
{
    Semaphore semaphore = {};
    semaphore.impl = reinterpret_cast<Core::Opaque*>(
        CreateSemaphoreA(nullptr, LONG(initial_value), Core::MaxValue<i32>, nullptr)
    );
    DebugAssert(semaphore.impl != nullptr, "can't create a new semaphore");
    return semaphore;
}

void Semaphore::destroy()
{
    CloseHandle(reinterpret_cast<HANDLE>(impl));
}

void Semaphore::signal()
{
    ReleaseSemaphore(reinterpret_cast<HANDLE>(impl), 1, nullptr);
}

void Semaphore::wait()
{
    WaitForSingleObject(reinterpret_cast<HANDLE>(impl), INFINITE);
}
