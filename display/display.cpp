#include "display/display.h"

#if defined(ENGINE_ANDROID)
#include "platform/android/android_display.h"
#elif defined(ENGINE_WIN32)
#include "platform/win32/win32_display.h"
#endif

static inline Display::VTable _get_display_vtable()
{
#if defined(ENGINE_ANDROID)
    return AndroidDisplay::get_vtable();
#elif defined(ENGINE_WIN32)
    return Win32Display::get_vtable();
#endif
}

void Display::initialize(const mem::Allocator& allocator)
{
    data.allocator = allocator;
    vtable = _get_display_vtable();
    vtable.initialize(allocator);
}
