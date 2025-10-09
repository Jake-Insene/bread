#include "platform/win32/win32_display.h"

#include "engine/engine.h"
#include "graphics/graphics.h"
#include "input/input.h"
#include "math/values.h"
#include "scene/scene_manager.h"


static inline Win32Display::WindowData& _get_window_data(Display::WindowID id)
{
	return Win32Display::data.windows.get(id);
}

static inline LRESULT WINAPI _default_window_proc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam)
{
	WindowID window_id = GetWindowLongPtrA(handle, GWLP_USERDATA) & math::MaxValue<Display::WindowID>;

	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return true;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	{
		MouseButton button =
			(msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP) ? MOUSE_BUTTON_LEFT
			: (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP) ? MOUSE_BUTTON_RIGHT
			: MOUSE_BUTTON_MIDDLE;

		Input::data.mouse_buttons[button] =
			msg == WM_LBUTTONDOWN
			|| msg == WM_RBUTTONDOWN
			|| msg == WM_MBUTTONDOWN;

		// Y positive is up
		Vector2 pos = Vector2(
			f32(GET_X_LPARAM(lparam)),
			-f32(GET_Y_LPARAM(lparam))
		);

		InputEventMouseButton event{};
		event.type = INPUT_EVENT_MOUSE_BUTTON;
		event.position = pos;
		event.pressed = Input::data.mouse_buttons[button];
		event.button = button;
		Engine::handle_input(event);
	}
	break;
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
		//u16 repeat_count = lparam & 0xFFFF;
		u8 scan_code = (lparam >> 16) & 0xFF;

		if (Input::data.keys[wparam] == KeyState::RequestNewState
			&& (msg == WM_SYSKEYDOWN || msg == WM_KEYDOWN))
		{
			break;
		}

		// Extended key
		// Shift, Ctr, Alt
		KeyState new_key_state = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) ?
			KeyState::Pressed : KeyState::Released;

		UINT real_vk = wparam & 0xFFFU;
		if (wparam == VK_SHIFT || wparam == VK_CONTROL || wparam == VK_MENU)
		{
			real_vk = MapVirtualKeyExA(scan_code, MAPVK_VSC_TO_VK_EX, GetKeyboardLayout(0));
		}

		Input::data.keys[real_vk] = new_key_state;
		Input::data.keys[wparam] = new_key_state;

		InputEventKey event = {};
		event.type = InputEventType::INPUT_EVENT_KEY;
		event.pressed = Input::data.keys[wparam] == KeyState::Pressed;
		event.key = (Key)wparam;

		Engine::handle_input(event);
	}
	break;
	case WM_SIZE:
	{
		Win32Display::WindowData& window_data = _get_window_data(window_id);
		if (window_id == Engine::data.main_window.window_id)
		{
			Engine::request_recreate_window();
		}

		GetWindowRect(handle, &window_data.window_rect);
	}
	break;
	case WM_MOUSEMOVE:
	{
		Vector2 screen_space_position = Vector2(
			f32(GET_X_LPARAM(lparam)), -f32(GET_Y_LPARAM(lparam))
		);
		Input::data.mouse_position = SceneManager::_screen_make_local_to_canvas(screen_space_position);
	}
	break;
	}

	return DefWindowProcA(handle, msg, wparam, lparam);
}

void Display::initialize(const mem::Allocator& allocator)
{
	Win32Display::data.allocator = allocator;
	Win32Display::data.windows = FreeList<Win32Display::WindowData, Display::WindowID>::with_size(allocator, 4);

	WNDCLASSEXA wc = {};
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = _default_window_proc;
	wc.lpszClassName = Win32Display::WindowClassName;
	wc.hIcon = LoadIconA(0, IDI_APPLICATION);
	wc.hCursor = LoadCursorA(0, IDC_ARROW);
	wc.hIconSm = LoadIconA(0, IDI_APPLICATION);

	RegisterClassExA(&wc);

	GetClientRect(GetDesktopWindow(), &Win32Display::data.fullscreen_rect);
}

void Display::shutdown()
{
	Win32Display::data.windows.destroy();
}

Display::WindowID Display::window_create()
{
	Display::WindowID new_id = Win32Display::data.windows.add(Win32Display::WindowData());
	Win32Display::WindowData& new_window = _get_window_data(new_id);

	RECT window_rect = {};
	window_rect.right = Display::DefaultWidth;
	window_rect.bottom = Display::DefaultHeight;

	AdjustWindowRectExForDpi(&window_rect, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW, GetDpiForSystem());

	new_window.window_rect = window_rect;
	new_window.handle = CreateWindowExA(
		WS_EX_OVERLAPPEDWINDOW, Win32Display::WindowClassName, Display::DefaultTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, window_rect.right - window_rect.left, window_rect.bottom - window_rect.top,
		0, 0, GetModuleHandleA(nullptr), 0
	);

	SetWindowLongPtrA(new_window.handle, GWLP_USERDATA, (LONG_PTR)new_id);

	ShowWindow(new_window.handle, SW_SHOW);

	return new_id;
}

Vector2I Display::window_get_size(Display::WindowID wid)
{
	Win32Display::WindowData& window_data = _get_window_data(wid);
	RECT rect;
	GetClientRect(window_data.handle, &rect);

	return Vector2I(
		rect.right - rect.left,
		rect.bottom - rect.top
	);
}

void Display::window_set_size(Display::WindowID wid, const Vector2I& new_size)
{
	Win32Display::WindowData& window_data = _get_window_data(wid);

	RECT rect = { 0, 0, new_size.x, new_size.y };
	AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);

	i32 width = rect.right - rect.left;
	i32 height = rect.bottom - rect.top;

	MoveWindow(
		window_data.handle, window_data.window_rect.left, window_data.window_rect.top,
		width, height, TRUE
	);
}

void* Display::window_get_native_handle(Display::WindowID wid)
{
	Win32Display::WindowData& window_data = _get_window_data(wid);
	return (void*)window_data.handle;
}


