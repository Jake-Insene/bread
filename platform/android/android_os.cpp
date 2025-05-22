#include "platform/android/android_os.h"


void OS::initialize()
{}

void OS::shutdown()
{}

void OS::exit(u64)
{}

usize OS::get_page_size()
{
    return (usize)sysconf(_SC_PAGESIZE);
}

OS::ThreadID OS::thread_create(ThreadFn, void*)
{
    return OS::ThreadID::InvalidID;
}
void OS::thread_destroy(ThreadID)
{}
bool OS::thread_join(ThreadID)
{
    return false;
}

OS::MutexID OS::mutex_create()
{
    return OS::MutexID::InvalidID;
}
void OS::mutex_destroy(ThreadID)
{}
void OS::mutex_lock(ThreadID)
{}
bool OS::mutex_try_lock(MutexID)
{
    return false;
}
void OS::mutex_unlock(MutexID)
{}

bool OS::set_current_directory(StringView)
{
    return true;
}