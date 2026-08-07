#include "platform/platform_header.h"

#include "engine/engine.h"
#include "debug/log.h"
#include "mem/generic_allocator.h"
#include "platform/win32/win32_engine.h"


extern "C"
{
	__declspec(dllexport)
	int _fltused = 0;
	
	unsigned long _tls_index = 0;

	__declspec(dllexport)
	void __chkstk() {}

	__declspec(dllexport)
	int __cdecl _purecall()
	{
		Log::error("Pure virtual function called!");
		return 0;
	}

	#pragma function(memset)
	void* __cdecl memset(void* dest, int c, size_t count)
	{
		PlatformIntricics::set(
			Slice(reinterpret_cast<u8*>(dest), count),
			static_cast<u8>(c)
		);
		return dest;
	}

	#pragma function(memcpy)
	void* __cdecl memcpy(void* dest, const void* src, size_t count)
	{
		Mem::copy(
			Slice(reinterpret_cast<u8*>(dest), count),
			Slice(reinterpret_cast<const u8*>(src), count)
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
	while (StackWalk64(machine, process, thread, &frame, &new_context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)
		!= 0)
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
		symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
		symbol->MaxNameLength = 254;

		if (SymGetSymFromAddr64(process, frame.AddrPC.Offset, NULL, symbol) != 0)
		{
			function_name = StringView(symbol->Name, __string_len(symbol->Name));
		}

		DWORD offset = 0;
		IMAGEHLP_LINE64 line_hlp = {};
		line_hlp.SizeOfStruct = sizeof(IMAGEHLP_LINE);
		if (SymGetLineFromAddr64(process, frame.AddrPC.Offset, &offset, &line_hlp) != 0)
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

alignas(alignof(Win32Engine)) static u8 place_holder_memory[sizeof(Win32Engine)]{};

static Win32Engine& get_engine()
{
	return *reinterpret_cast<Win32Engine*>(place_holder_memory);
}

void engine_loop(Mem::Allocator& allocator)
{
	Engine::local_data.engine_runtime = &get_engine();
	ConstructObject(get_engine(), allocator);

	bool quit = false;
	while(quit == false)
	{
		MSG msg;

		get_engine().pre_step();
		while(PeekMessageA(&msg, 0, 0, 0, PM_REMOVE) != 0)
		{
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
			if (msg.message == WM_QUIT)
			{
				quit = true;
				break;
			}
		}

		if(quit)
		{
			break;
		}

		get_engine().step();
	}

	DestructObject(get_engine());
}

// Default for Windows
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	SetUnhandledExceptionFilter(&_exception_handler);

	// TODO: Accessing engine before initialization!
	bool enable_console = __get_application_info__().enable_debug_console;

	if (enable_console)
	{
		if (AttachConsole(ATTACH_PARENT_PROCESS) == FALSE)
		{
			AllocConsole();
		}
	}

	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	{
		Mem::GenericAllocator global_allocator;
		Main::runtime_begin(global_allocator);
		engine_loop(global_allocator);
		Main::runtime_end();
	}
	CoUninitialize();

	if(enable_console)
	{
		u8 bytes[2] = {};
		Format::format<false>(IO::File::get_stdout().writer(), "Press enter to close the console...");
		IO::File::get_stdin().read(bytes);
	}

	ExitProcess(0);
}


