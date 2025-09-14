#include "platform/platform_header.h"

#include "platform/win32/win32_engine.h"
#include <intrin.h>

#if defined(_MSC_VER)
extern "C" {
	int _fltused = 0;
	__declspec(selectany) unsigned long _tls_index = 0;

	extern "C" void __chkstk(void) {}

	#pragma function(memset)
	void* __cdecl __bread_memset(void* dest, int c, size_t count)
	{
		unsigned char* p = (unsigned char*)dest;
		while (count--)
		{
			*p++ = (unsigned char)c;
		}
		return dest;
	}

	// memcpy
	#pragma function(memcpy)
	void* __cdecl __bread_memcpy(void* dest, const void* src, size_t count)
	{
		unsigned char* d = (unsigned char*)dest;
		const unsigned char* s = (const unsigned char*)src;
		while (count--)
		{
			*d++ = *s++;
		}
		return dest;
	}
}

#endif


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	if(Engine::get_configuration().enable_debug_console)
		AllocConsole();

	Win32Engine::initialize();

	while (true)
	{
		MSG msg;
		if (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
			if (msg.message == WM_QUIT)
				break;
		}
		
		Win32Engine::step();
	}

	Win32Engine::shutdown();

	ExitProcess(0);
}


