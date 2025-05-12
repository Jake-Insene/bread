#include "os/os.h"

#if defined(ENGINE_ANDROID)
#include "platform/android/android_os.h"
#elif defined(ENGINE_WIN32)
#include "platform/win32/win32_os.h"
#endif

static inline OS::VTable _get_os_vtable()
{
#if defined(ENGINE_ANDROID)
    return AndroidOS::get_vtable();
#elif defined(ENGINE_WIN32)
    return Win32OS::get_vtable();
#endif
}

void OS::initialize()
{
    vtable = _get_os_vtable();
    vtable.initialize();
}
