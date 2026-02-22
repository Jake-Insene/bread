#include "platform/platform_header.h"

#include "log/log.h"
#include "platform/win32/win32_engine.h"


extern "C"
{
	__declspec(dllexport)
	int _fltused = 0;
	
	unsigned long _tls_index = 0;

	__declspec(dllexport)
	void __chkstk() {}

	#pragma function(memset)
	void* __cdecl memset(void* dest, int c, size_t count)
	{
		if(c == 0)
		{
			PlatformIntricics::setzero(Slice<u8>(
				reinterpret_cast<u8*>(dest), count)
			);
		}
		else
		{
			mem::set(
				Slice<u8>(reinterpret_cast<u8*>(dest), count), u8(c)
			);
		}
		return dest;
	}

	#pragma function(memcpy)
	void* __cdecl memcpy(void* dest, const void* src, size_t count)
	{
		mem::copy(
			Slice<u8>(reinterpret_cast<u8*>(dest), count),
			Slice<const u8>(reinterpret_cast<const u8*>(src), count)
		);
		return dest;
	}

	#pragma function(strlen)
	usize __cdecl strlen(const char* str)
	{
		return __string_len(str);
	}
}

LONG _exception_handler(EXCEPTION_POINTERS* ep)
{
	switch (ep->ExceptionRecord->ExceptionCode)
	{
	case EXCEPTION_BREAKPOINT:
		Log::error("Breakpoint was reached:");
		break;
	default:
		Log::error(
			"Exception at address {} with code {}:",
			ep->ExceptionRecord->ExceptionAddress,
			ep->ExceptionRecord->ExceptionCode
		);
		break;
	}

	HANDLE process = GetCurrentProcess();
	HANDLE thread = GetCurrentThread();
	DWORD machine = IMAGE_FILE_MACHINE_AMD64;

	SymInitialize(process, NULL, TRUE);
	SymSetOptions(SYMOPT_LOAD_LINES);

	STACKFRAME64 frame = {};
	frame.AddrPC.Offset = ep->ContextRecord->Rip;
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrFrame.Offset = ep->ContextRecord->Rbp;
	frame.AddrFrame.Mode = AddrModeFlat;
	frame.AddrStack.Offset = ep->ContextRecord->Rsp;
	frame.AddrStack.Mode = AddrModeFlat;

	StringView module_name = StringView();
	StringView function_name = StringView();
	StringView file_name = StringView();
	char module_name_buff[260]{};

	CONTEXT new_context = *ep->ContextRecord;
	while (StackWalk64(machine, process, thread, &frame, &new_context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
	{
		DWORD line = 0;

		DWORD64 module_base = SymGetModuleBase64(process, frame.AddrPC.Offset);
		DWORD module_name_len = GetModuleFileNameA(
			reinterpret_cast<HINSTANCE>(module_base), module_name_buff, MAX_PATH
		);
		module_name = StringView(module_name_buff, module_name_len);

		usize index = module_name_len - 1;
		while (module_name[index] != '\\' && index > 0)
		{
			index--;
		}

		module_name = StringView(module_name_buff + index + 1, module_name_len - index - 1);

		char symbol_buffer[sizeof(IMAGEHLP_SYMBOL64) + 255];
		PIMAGEHLP_SYMBOL64 symbol = reinterpret_cast<PIMAGEHLP_SYMBOL64>(symbol_buffer);
		symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64) + 255;
		symbol->MaxNameLength = 254;

		if (SymGetSymFromAddr64(process, frame.AddrPC.Offset, NULL, symbol))
		{
			function_name = StringView(symbol->Name, __string_len(symbol->Name));
		}

		DWORD offset = 0;
		IMAGEHLP_LINE64 line_hlp = {};
		line_hlp.SizeOfStruct = sizeof(IMAGEHLP_LINE);
		if (SymGetLineFromAddr64(process, frame.AddrPC.Offset, &offset, &line_hlp))
		{
			file_name = StringView(line_hlp.FileName, __string_len(line_hlp.FileName));
			line = line_hlp.LineNumber;

			index = file_name.len - 1;
			while (file_name[index] != '\\' && index > 0)
			{
				index--;
			}

			file_name = StringView(file_name.items + index + 1, file_name.len - index - 1);
		}

		Log::error("\t{}: {} at line {}: {}", file_name, function_name, line, module_name);
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

void engine_loop()
{
	if (Engine::get_configuration().enable_debug_console)
	{
		if (!AttachConsole(ATTACH_PARENT_PROCESS))
		{
			AllocConsole();
		}
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
}

// Default for Windows
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	AddVectoredExceptionHandler(1, &_exception_handler);
	engine_loop();
	ExitProcess(0);
}


