#include "platform/platform_header.h"

#include "platform/win32/win32_engine.h"
#include "objects/scene_manager.h"

static inline bool can_recreate = false;

static LRESULT handle_proc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return true;
	case WM_LBUTTONDOWN:
	{
		// Y positive is up
		Vector2 pos
		{
			(f32)GET_X_LPARAM(lparam),
			-(f32)GET_Y_LPARAM(lparam)
		};

		InputEventMouseButton event{};
		event.type = INPUT_EVENT_MOUSE_BUTTON;
		event.position = pos;
		event.left = true;
		Win32Engine::handle_input(event);
	}
	break;
	case WM_LBUTTONUP:
	{
		Vector2 pos
		{
			(f32)GET_X_LPARAM(lparam),
			(f32)GET_Y_LPARAM(lparam)
		};

		InputEventMouseButton event{};
		event.type = INPUT_EVENT_MOUSE_BUTTON;
		event.position = pos;
		Win32Engine::handle_input(event);
	}
		break;
	case WM_KEYDOWN:
	{
		Input::data.keys[wparam] = true;
	}
	break;
	case WM_KEYUP:
	{
		Input::data.keys[wparam] = false;
	}
		break;
	case WM_SIZE:
	{
		if(can_recreate && Win32Engine::window != nullptr)
		{
			Engine::recreate_window();
		}
	}
		break;
	}

	return DefWindowProcA(window, msg, wparam, lparam);
}

int WINAPI wWinMain(HINSTANCE hinstance, HINSTANCE previnstance, LPWSTR cmdline, int showcmd)
{
	SetProcessDPIAware();
	if (!SetCurrentDirectoryA("assets"))
	{
		MessageBoxA(0, "Couldn't set the current directory", "Error", MB_OK);
		return 1;
	}

	char current_directory[256]{};
	GetCurrentDirectoryA(256, current_directory);

	WNDCLASSEXA wc = {};
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = handle_proc;
	wc.lpszClassName = "window";
	wc.hIcon = LoadIconA(0, IDI_APPLICATION);
	wc.hCursor = LoadCursorA(0, IDC_ARROW);
	wc.hIconSm = LoadIconA(0, IDI_APPLICATION);

	RegisterClassExA(&wc);

	RECT window_rect = {};
	window_rect.right = SceneManager::DefaultWidth;
	window_rect.bottom = SceneManager::DefaultHeight;

	AdjustWindowRectExForDpi(&window_rect, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW, GetDpiForSystem());

	Win32Engine::window = CreateWindowExA(
		WS_EX_OVERLAPPEDWINDOW, "window", "Bread", WS_OVERLAPPEDWINDOW, 
		CW_USEDEFAULT, CW_USEDEFAULT, window_rect.right - window_rect.left, window_rect.bottom - window_rect.top,
		0, 0, hinstance, 0
	);

	ShowWindow(Win32Engine::window, showcmd);

	Win32Engine::initialize();
	can_recreate = true;

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


