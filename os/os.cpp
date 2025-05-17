#include "os/os.h"

#if defined(ENGINE_ANDROID)
#include "platform/android/android_os.h"
#elif defined(ENGINE_WIN32)
#include "platform/win32/win32_os.h"
#endif

