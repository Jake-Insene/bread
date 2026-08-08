#include "platform/win32/win32_display.h"

#include "engine/engine.h"
#include "input/input.h"


static inline Win32Display::WindowData& _get_window_data(Display::WindowID id);

static inline LRESULT WINAPI _default_window_proc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam)
{
	Display::WindowID window_id = Display::WindowID(
		GetWindowLongPtrA(handle, GWLP_USERDATA) & MaxValue<Display::WindowID::Type>
	);

	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_CLOSE:
	{
		InputEventWindowClose event = {};
		event.type = EventType::WindowClose;
		event.window = window_id;
		Engine::local_data.engine_runtime->handle_event(event);
	}
		break;
	case WM_CAPTURECHANGED:
	{
		Win32Display::WindowData& window_data = _get_window_data(window_id);
		
		if (window_data.mouse_captured)
		{
			window_data.mouse_captured = false;
		}
		break;
	}
	case WM_KILLFOCUS:
	case WM_CANCELMODE:
	case WM_MOUSELEAVE:
	{
		Input::update_mouse_button(MouseButton::Left, false);
		Input::update_mouse_button(MouseButton::Middle, false);
		Input::update_mouse_button(MouseButton::Right, false);

		MouseButton buttons[] = {MouseButton::Left, MouseButton::Middle, MouseButton::Right};
		for(MouseButton btn : buttons)
		{
			Input::data.mouse_buttons[i32(btn)] = false;

			InputEventMouseButton event = {};
			event.type = EventType::MouseButton;
			event.position = Input::get_mouse_position();
			event.pressed = false;
			event.button = btn;
			Engine::local_data.engine_runtime->handle_event(event);
		}
	}
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	{
		MouseButton button = MouseButton::Middle;
		if(msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP)
		{
			button = MouseButton::Left;
		}
		else if(msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP)
		{
			button = MouseButton::Right;
		}

		Input::update_mouse_button(button,
			msg == WM_LBUTTONDOWN
			|| msg == WM_RBUTTONDOWN
			|| msg == WM_MBUTTONDOWN);

		if(Input::get_mouse_state(button))
		{
			Win32Display::WindowData& window_data = _get_window_data(window_id);

			SetCapture(handle);
    		window_data.mouse_captured = true;
		}
		else
		{
			Win32Display::WindowData& window_data = _get_window_data(window_id);

			if(window_data.mouse_captured)
			{
				ReleaseCapture();
				window_data.mouse_captured = false;
			}
		}

		// Getting bread screen space coordinates
		RECT client_rect;
		GetClientRect(handle, &client_rect);

		Vector2 screen_space_position = Vector2(
			f32(GET_X_LPARAM(lparam)),
			(client_rect.bottom - client_rect.top) - f32(GET_Y_LPARAM(lparam))
		);

		InputEventMouseButton event = {};
		event.type = EventType::MouseButton;
		event.position = screen_space_position;
		event.pressed = Input::get_mouse_state(button);
		event.button = button;
		Engine::local_data.engine_runtime->handle_event(event);
		return 0;
	}
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
		//u16 repeat_count = lparam & 0xFFFF;
		u8 scan_code = (lparam >> 16) & 0xFF;

		if (Input::get_key_state(Key(wparam)) == KeyState::RequestNewState
			&& (msg == WM_SYSKEYDOWN || msg == WM_KEYDOWN))
		{
			return 0;
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

		Input::update_key_state(Key(real_vk), new_key_state);
		Input::update_key_state(Key(wparam), new_key_state);

		InputEventKey event = {};
		event.type = EventType::KeyPress;
		event.pressed = Input::get_key_state(Key(wparam)) == KeyState::Pressed;
		event.key = static_cast<Key>(wparam);

		Engine::local_data.engine_runtime->handle_event(event);
		return 0;
	}
	break;
	case WM_SIZE:
	{
		Win32Display::WindowData& window_data = _get_window_data(window_id);
		GetWindowRect(handle, &window_data.window_rect);

		InputEventWindowResize event = {};
		event.type = EventType::WindowResize;
		event.window = window_id;
		event.size = Vector2I(
			window_data.window_rect.right - window_data.window_rect.left,
			window_data.window_rect.bottom - window_data.window_rect.top
		);
		
		Engine::local_data.engine_runtime->handle_event(event);

		return 0;
	}
	break;
	case WM_MOUSEMOVE:
	{
		RECT client_rect;
		GetClientRect(handle, &client_rect);

		Vector2 screen_space_position = Vector2(
			f32(GET_X_LPARAM(lparam)),
			(client_rect.bottom - client_rect.top) - f32(GET_Y_LPARAM(lparam))
		);
		Input::data.mouse_position = screen_space_position;

		InputEventMouseMove event = {};
		event.type = EventType::MouseMove;
		event.position = screen_space_position;
		Engine::local_data.engine_runtime->handle_event(event);
		return 0;
	}
	break;
	}

	return DefWindowProcA(handle, msg, wparam, lparam);
}

alignas(alignof(Win32Display::InternalData)) static u8 place_holder_memory[sizeof(Win32Display::InternalData)]{};
static Win32Display::InternalData& get_data()
{
	return *reinterpret_cast<Win32Display::InternalData*>(place_holder_memory);
}

static inline Win32Display::WindowData& _get_window_data(Display::WindowID id)
{
	return get_data().windows.get(id);
}

void Display::initialize(Mem::Allocator& allocator)
{
	// Ensures constructors are call.
    Core::Mem::Placement(get_data(), allocator);

	WNDCLASSEXA wc = {};
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = _default_window_proc;
	wc.lpszClassName = Win32Display::WindowClassName;
	wc.hIcon = LoadIconA(0, IDI_APPLICATION);
	wc.hCursor = LoadCursorA(0, IDC_ARROW);
	wc.hIconSm = LoadIconA(0, IDI_APPLICATION);
	RegisterClassExA(&wc);

	GetClientRect(GetDesktopWindow(), &get_data().fullscreen_rect);
}

void Display::shutdown()
{
	Core::Mem::Destruct(get_data());
}

Display::WindowID Display::window_create()
{
	Display::WindowID new_id = get_data().windows.add(Win32Display::WindowData());
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

	SetWindowLongPtrA(new_window.handle, GWLP_USERDATA, static_cast<LONG_PTR>(new_id.integer()));

	ShowWindow(new_window.handle, SW_SHOW);

	new_window.surface = GPU::surface_create({.window_native_handle = window_get_native_handle(new_id)});

	return new_id;
}

void Display::window_destroy(WindowID window_id)
{
	Win32Display::WindowData& window_data = _get_window_data(window_id);

	GPU::surface_destroy(window_data.surface);
	
	DestroyWindow(window_data.handle);

	get_data().windows.remove(window_id);
}

Vector2I Display::window_get_size(Display::WindowID window_id)
{
	Win32Display::WindowData& window_data = _get_window_data(window_id);
	RECT rect;
	GetClientRect(window_data.handle, &rect);

	return Vector2I(
		rect.right - rect.left,
		rect.bottom - rect.top
	);
}

void Display::window_set_size(Display::WindowID window_id, const Vector2I& new_size)
{
	Win32Display::WindowData& window_data = _get_window_data(window_id);

	RECT rect = { 0, 0, new_size.x, new_size.y };
	AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);

	i32 width = rect.right - rect.left;
	i32 height = rect.bottom - rect.top;

	MoveWindow(
		window_data.handle, window_data.window_rect.left, window_data.window_rect.top,
		width, height, TRUE
	);
}

MemoryAddress Display::window_get_native_handle(Display::WindowID window_id)
{
	Win32Display::WindowData& window_data = _get_window_data(window_id);
	return MemoryAddress(window_data.handle);
}

GPU::SurfaceID Display::window_get_surface(WindowID window_id)
{
	Win32Display::WindowData& window_data = _get_window_data(window_id);
	return window_data.surface;
}

