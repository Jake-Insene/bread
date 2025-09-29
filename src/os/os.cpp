#include "os/os.h"


#if defined(BREAD_WIN32)
#include "platform/win32/win32_os.h"
using PlatformOS = Win32OS;
#endif


void OS::initialize(const mem::Allocator& allocator)
{
	PlatformOS::initialize(allocator);
}

void OS::shutdown()
{
	PlatformOS::shutdown();
}

void OS::exit(u64 code)
{
	PlatformOS::exit(code);
}

usize OS::get_page_size()
{
	return PlatformOS::get_page_size();
}

OS::ThreadID OS::thread_create(ThreadFn fn, void* arg)
{
	return PlatformOS::thread_create(fn, arg);
}

void OS::thread_destroy(ThreadID tid)
{
	PlatformOS::thread_destroy(tid);
}

bool OS::thread_join(ThreadID tid)
{
	return PlatformOS::thread_join(tid);
}

void OS::thread_set_name(ThreadID tid, StringView new_name)
{
	PlatformOS::thread_set_name(tid, new_name);
}

StringView OS::thread_get_name(ThreadID tid)
{
	return PlatformOS::thread_get_name(tid);
}

OS::MutexID OS::mutex_create()
{
	return PlatformOS::mutex_create();
}

void OS::mutex_destroy(MutexID mid)
{
	PlatformOS::mutex_destroy(mid);
}

void OS::mutex_lock(MutexID mid)
{
	PlatformOS::mutex_lock(mid);
}

bool OS::mutex_try_lock(MutexID mid)
{
	return PlatformOS::mutex_try_lock(mid);
}

void OS::mutex_unlock(MutexID mid)
{
	PlatformOS::mutex_unlock(mid);
}

OS::SemaphoreID OS::semaphore_create(usize initial_value)
{
	return PlatformOS::semaphore_create(initial_value);
}

void OS::semaphore_destroy(SemaphoreID sid)
{
	PlatformOS::semaphore_destroy(sid);
}

void OS::semaphore_signal(SemaphoreID sid)
{
	PlatformOS::semaphore_signal(sid);
}

void OS::semaphore_wait(SemaphoreID sid)
{
	PlatformOS::semaphore_wait(sid);
}

bool OS::set_current_directory(StringView dir)
{
	return PlatformOS::set_current_directory(dir);
}

