#pragma once

#if defined(ENGINE_ANDROID)
#include "platform/android/android_header.h"
#elif defined(ENGINE_WIN32)
#include "platform/win32/win32_header.h"
#endif // ENGINE_ANDROID
