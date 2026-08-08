#pragma once
#include "Core/Header.h"


#if defined(BREAD_ANDROID)
#include "platform/android/android_header.h"
#elif defined(BREAD_WIN32)
#include "platform/win32/win32_header.h"
#endif
