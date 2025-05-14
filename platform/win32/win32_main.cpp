#include "platform/platform_header.h"

#include "platform/win32/win32_engine.h"
#include "objects/scene_manager.h"


int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
	if (!SetCurrentDirectoryA("assets"))
	{
		MessageBoxA(0, "Assets directory not found", "Error", MB_OK);
		return 1;
	}

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

	return 0;
}


