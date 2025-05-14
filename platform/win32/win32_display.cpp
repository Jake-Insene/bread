#include "platform/win32/win32_display.h"

#include "engine/engine.h"
#include "io/input.h"
#include "objects/scene_manager.h"


static inline Win32Display::WindowData& _get_window_data(Display::WindowID id)
{
    return Win32Display::data.windows.get(id);
}

static inline LRESULT WINAPI _default_window_proc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam)
{
	WindowID window_id = (Display::WindowID)GetWindowLongPtrA(handle, GWLP_USERDATA);

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
		Vector2 pos
		{
			(f32)GET_X_LPARAM(lparam),
			-(f32)GET_Y_LPARAM(lparam)
		};

		InputEventMouseButton event{};
		event.type = INPUT_EVENT_MOUSE_BUTTON;
		event.position = pos;
		event.pressed = Input::data.mouse_buttons[button];
		event.button = button;
		Engine::handle_input(event);
	}
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
		Input::data.keys[wparam] = msg == WM_KEYDOWN;

		InputEventKey event{};
		event.pressed = Input::data.keys[wparam];
		event.key = (Key)wparam;
		
		Engine::handle_input(event);
	}
	break;
	case WM_SIZE:
	{
		Win32Display::WindowData& window_data = _get_window_data(window_id);
		if (window_id == Engine::data.main_window.window_id)
		{
			Engine::recreate_window();
		}

		GetWindowRect(handle, &window_data.window_rect);
	}
	break;
	case WM_MOUSEMOVE:
	{
		Vector2 screen_space_position = Vector2(GET_X_LPARAM(lparam), -GET_Y_LPARAM(lparam));
		Input::data.mouse_position = SceneManager::_screen_make_local(screen_space_position);
	}
		break;
	}

	return DefWindowProcA(handle, msg, wparam, lparam);
}

Display::VTable Win32Display::get_vtable()
{
    return Display::VTable
    {
        .initialize = &Win32Display::initialize,
        .shutdown = &Win32Display::shutdown,

		.window_create = &Win32Display::window_create,
		.window_get_size = &Win32Display::window_get_size,
		.window_set_size = &Win32Display::window_set_size,
		.window_get_native_handle = &Win32Display::window_get_native_handle,
    };
}

void Win32Display::initialize(const mem::Allocator& allocator)
{
    data.allocator = allocator;
    data.windows = QueueArray<WindowData, Display::WindowID>::with_size(allocator, 4);

    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = _default_window_proc;
    wc.lpszClassName = WindowClassName;
    wc.hIcon = LoadIconA(0, IDI_APPLICATION);
    wc.hCursor = LoadCursorA(0, IDC_ARROW);
    wc.hIconSm = LoadIconA(0, IDI_APPLICATION);

    RegisterClassExA(&wc);

	GetClientRect(GetDesktopWindow(), &data.fullscreen_rect);
}

void Win32Display::shutdown()
{
    data.windows.destroy();
}

Display::WindowID Win32Display::window_create()
{
    Display::WindowID new_id = data.windows.add(WindowData());
    WindowData& new_window = _get_window_data(new_id);

    RECT window_rect = {};
    window_rect.right = Display::DefaultWidth;
    window_rect.bottom = Display::DefaultHeight;

    AdjustWindowRectExForDpi(&window_rect, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW, GetDpiForSystem());

    new_window.window_rect = window_rect;
    new_window.handle = CreateWindowExA(
        WS_EX_OVERLAPPEDWINDOW, WindowClassName, Display::DefaultTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, window_rect.right - window_rect.left, window_rect.bottom - window_rect.top,
        0, 0, GetModuleHandleA(nullptr), 0
    );

	SetWindowLongPtrA(new_window.handle, GWLP_USERDATA, (LONG_PTR)new_id);

    ShowWindow(new_window.handle, SW_SHOW);

    return new_id;
}

Vector2I Win32Display::window_get_size(Display::WindowID wid)
{
	WindowData& window_data = _get_window_data(wid);
	RECT rect;
	GetClientRect(window_data.handle, &rect);

    return Vector2I(
		rect.right - rect.left,
		rect.bottom - rect.top
	);
}

void Win32Display::window_set_size(Display::WindowID wid, const Vector2I& new_size)
{
	WindowData& window_data = _get_window_data(wid);

	RECT rect = { 0, 0, new_size.x, new_size.y };
	AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);

	i32 width = rect.right - rect.left;
	i32 height = rect.bottom - rect.top;

	MoveWindow(
		window_data.handle, window_data.window_rect.left, window_data.window_rect.top,
		width, height, TRUE
	);
}

void* Win32Display::window_get_native_handle(Display::WindowID wid)
{
	WindowData& window_data = _get_window_data(wid);
	return (void*)window_data.handle;
}


