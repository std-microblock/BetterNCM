#include "pch.h"
#include "ErrorHandler.h"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Text_Buffer.H>
#include <functional>
#include <string>
#include <ostream>

#include <dbghelp.h>

#include "PluginManager.h"
#include "BetterNCMNativePlugin.h"

#include "utils/BNString.hpp"
#include "utils/utils.h"

extern NCMProcessType process_type;

inline const char* u8(const char8_t* s) {
    return reinterpret_cast<const char*>(s);
}

void show_error_dialog(ErrorDialogParams params) {
    // Init window
    Fl_Window* window = new Fl_Window(500, 650, "BetterNCM Crashed!");

    // Display error message
    Fl_Box* error_title = new Fl_Box(0, 5, window->w(), 40, u8(u8"抱歉, 网易云音乐崩溃了!"));
    error_title->labelsize(20);
    error_title->align(FL_ALIGN_CENTER);

    Fl_Box* error_desc = new Fl_Box(0, 35, window->w(), 40, u8(u8"这有可能是因为 BetterNCM 的原因"));
    error_desc->labelsize(13);
    error_desc->align(FL_ALIGN_CENTER);

    // Display error type and process id
    Fl_Box* error_detail = new Fl_Box(20, 70, window->w() - 40, 20,
		(new std::string(params.error_type + ", Proc:" + std::to_string(params.error_process_id)))->c_str()
		);

    // Display stack trace
    Fl_Group* text_group = new Fl_Group(20, 120, window->w() - 40, 100);
    Fl_Text_Buffer* text_buffer = new Fl_Text_Buffer();
    Fl_Text_Display* text_display = new Fl_Text_Display(20, 100, window->w() - 40, 400);
    text_display->buffer(text_buffer);
    text_group->end();
	text_display->textsize(8);
    text_buffer->text(params.stack_trace.c_str());

    // Display plugin name if available
    if (!params.plugin_name.empty()) {
		Fl_Box* probable_plugin = new Fl_Box(20, 520, window->w() - 40, 20,
			u8(u8"可能的出现问题的插件：")
		);

        Fl_Input* plugin_name_input = new Fl_Input(20, 540, window->w() - 40, 25);
        plugin_name_input->value(params.plugin_name.c_str());
    }

    // Display action buttons
    Fl_Button* restart_button = new Fl_Button(20, window->h() - 50, (window->w() - 40) / 2 - 5, 25,
        u8(u8"重启"));
    restart_button->callback([](Fl_Widget* w, void* data) {
        auto callback = reinterpret_cast<std::function<void()>*>(data);
        (*callback)();
        }, &params.restart_callback);

    Fl_Button* disable_restart_button = new Fl_Button(restart_button->x() + restart_button->w() + 10,
        restart_button->y(),
        (window->w() - 40) / 2 - 5, 25,
        u8(u8"禁用本体并重启"));
    disable_restart_button->callback([](Fl_Widget* w, void* data) {
        auto callback = reinterpret_cast<std::function<void()>*>(data);
        (*callback)();
        }, &params.disable_restart_callback);

    Fl_Button* close_button = new Fl_Button(20, restart_button->y() - restart_button->h() - 5,
        (window->w() - 40) / 2 - 5, 25, u8(u8"关闭"));
    close_button->callback([](Fl_Widget* w, void* data) {
        auto callback = reinterpret_cast<std::function<void()>*>(data);
        (*callback)();
        }, &params.close_callback);

    Fl_Button* fix_button =
        new Fl_Button(restart_button->x() + disable_restart_button->w() + 10, close_button->y(),
            (window->w() - 40) / 2 - 5, 25, u8(u8"尝试自动修复"));
    fix_button->callback([](Fl_Widget* w, void* data) {
        auto callback = reinterpret_cast<std::function<void()>*>(data);
        (*callback)();
        }, &params.fix_callback);
    fix_button->deactivate(); // Disable fix button by default

    if (params.show_fix_button) {
        fix_button->activate(); // Enable fix button if requested
    }

    
    window->resizable(0);
    window->color(fl_rgb_color(255, 255, 255));
    window->end();
    window->show();
    window->set_tooltip_window();
    Fl::run();
}



#include <string>
#include <wchar.h>
#include <DbgHelp.h>

#pragma comment(lib, "dbghelp.lib")

std::wstring GetBacktrace(const EXCEPTION_POINTERS* ExceptionInfo) {
	std::wstring info;
	wchar_t symbol_mem[sizeof(IMAGEHLP_SYMBOL64) + 256];
	auto symbol = (IMAGEHLP_SYMBOL64*)symbol_mem;

	// Get the context record from the exception information
	CONTEXT* contextRecord = ExceptionInfo->ContextRecord;

	// Initialize the symbol handler
	SymInitialize(GetCurrentProcess(), nullptr, TRUE);

	// Initialize the stack frame
	STACKFRAME64 stackFrame = { 0 };
	stackFrame.AddrPC.Offset = contextRecord->Rip;
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Offset = contextRecord->Rbp;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
	stackFrame.AddrStack.Offset = contextRecord->Rsp;
	stackFrame.AddrStack.Mode = AddrModeFlat;

	DWORD64 displacement = 0;
	// Walk the call stack and append each frame to the string
	while (StackWalk64(IMAGE_FILE_MACHINE_AMD64, GetCurrentProcess(), GetCurrentThread(), &stackFrame, contextRecord,
		nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr)) {
		// Get the module name and offset for this frame
		DWORD64 moduleBase = SymGetModuleBase64(GetCurrentProcess(), stackFrame.AddrPC.Offset);
		wchar_t moduleName[MAX_PATH];
		GetModuleFileNameW((HMODULE)moduleBase, moduleName, MAX_PATH);
		DWORD64 offset = stackFrame.AddrPC.Offset - moduleBase;

		symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
		symbol->MaxNameLength = 255;

		wchar_t name[256] = {};
		SymGetSymFromAddr64(GetCurrentProcess(), stackFrame.AddrPC.Offset, &displacement, symbol);
		UnDecorateSymbolNameW(BNString(symbol->Name).c_str(), name, 256, UNDNAME_COMPLETE);

		// Append the frame to the string
		wchar_t frameInfo[1024];
		swprintf_s(frameInfo, L"%s + %I64X", moduleName, offset);
		info += std::wstring(frameInfo) + std::wstring(L"(") + std::wstring(name) + std::wstring(L")\n");
	}

	// Cleanup the symbol handler
	SymCleanup(GetCurrentProcess());

	// Return the backtrace string
	return info;
}

std::string GetExceptionName(EXCEPTION_POINTERS* ExceptionInfo) {
	std::string exceptionName;
	switch (ExceptionInfo->ExceptionRecord->ExceptionCode) {
	case EXCEPTION_ACCESS_VIOLATION:
		exceptionName = "ACCESS_VIOLATION";
		break;
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		exceptionName = "ARRAY_BOUNDS_EXCEEDED";
		break;
	case EXCEPTION_BREAKPOINT:
		exceptionName = "BREAKPOINT";
		break;
	case EXCEPTION_DATATYPE_MISALIGNMENT:
		exceptionName = "DATATYPE_MISALIGNMENT";
		break;
	case EXCEPTION_FLT_DENORMAL_OPERAND:
		exceptionName = "FLT_DENORMAL_OPERAND";
		break;
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		exceptionName = "FLT_DIVIDE_BY_ZERO";
		break;
	case EXCEPTION_FLT_INEXACT_RESULT:
		exceptionName = "FLT_INEXACT_RESULT";
		break;
	case EXCEPTION_FLT_INVALID_OPERATION:
		exceptionName = "FLT_INVALID_OPERATION";
		break;
	case EXCEPTION_FLT_OVERFLOW:
		exceptionName = "FLT_OVERFLOW";
		break;
	case EXCEPTION_FLT_STACK_CHECK:
		exceptionName = "FLT_STACK_CHECK";
		break;
	case EXCEPTION_FLT_UNDERFLOW:
		exceptionName = "FLT_UNDERFLOW";
		break;
	case EXCEPTION_ILLEGAL_INSTRUCTION:
		exceptionName = "ILLEGAL_INSTRUCTION";
		break;
	case EXCEPTION_IN_PAGE_ERROR:
		exceptionName = "IN_PAGE_ERROR";
		break;
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		exceptionName = "INT_DIVIDE_BY_ZERO";
		break;
	case EXCEPTION_INT_OVERFLOW:
		exceptionName = "INT_OVERFLOW";
		break;
	case EXCEPTION_INVALID_DISPOSITION:
		exceptionName = "INVALID_DISPOSITION";
		break;
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		exceptionName = "NONCONTINUABLE_EXCEPTION";
		break;
	case EXCEPTION_PRIV_INSTRUCTION:
		exceptionName = "PRIV_INSTRUCTION";
		break;
	case EXCEPTION_SINGLE_STEP:
		exceptionName = "SINGLE_STEP";
		break;
	case EXCEPTION_STACK_OVERFLOW:
		exceptionName = "STACK_OVERFLOW";
		break;
	default:
		exceptionName = "UNKNOWN";
		break;
	}
	return exceptionName;
}

std::wstring PrintExceptionInfo(EXCEPTION_POINTERS* ExceptionInfo) {
	std::wostringstream Stream;

	EXCEPTION_RECORD* ExceptionRecord = ExceptionInfo->ExceptionRecord;
	CONTEXT* ContextRecord = ExceptionInfo->ContextRecord;

	Stream << L"Exception flags: 0x" << std::hex << ExceptionRecord->ExceptionFlags << std::endl;
	Stream << L"Exception address: 0x" << std::hex << ExceptionRecord->ExceptionAddress << std::endl;

#ifdef _WIN64
	Stream << L"  RIP: 0x" << std::hex << ContextRecord->Rip << std::endl;
	Stream << L"  RSP: 0x" << std::hex << ContextRecord->Rsp << std::endl;
	Stream << L"  RBP: 0x" << std::hex << ContextRecord->Rbp << std::endl;
#else
	Stream << L"  EIP: 0x" << std::hex << ContextRecord->Eip << std::endl;
	Stream << L"  ESP: 0x" << std::hex << ContextRecord->Esp << std::endl;
	Stream << L"  EBP: 0x" << std::hex << ContextRecord->Ebp << std::endl;
#endif


	Stream << L"Backtrace:\n  See console for detail" << std::endl;

	return Stream.str();
}

extern BNString datapath;

LONG WINAPI BNUnhandledExceptionFilter(EXCEPTION_POINTERS* ExceptionInfo) {
	HWND ncmWin = FindWindow(L"OrpheusBrowserHost", nullptr);

#define IGNORE_ERROR_EXIT(code) if(ExceptionInfo->ExceptionRecord->ExceptionCode==code){ util::killNCM(); return EXCEPTION_EXECUTE_HANDLER;}
#define IGNORE_ERROR_RESTART(code) if(ExceptionInfo->ExceptionRecord->ExceptionCode==code){ util::restartNCM(); return EXCEPTION_EXECUTE_HANDLER; }
#define IGNORE_ERROR_PASS_TO_NCM(code) if(ExceptionInfo->ExceptionRecord->ExceptionCode==code){ return EXCEPTION_CONTINUE_SEARCH; }
#define IGNORE_ERROR_AUTO(code) if(ExceptionInfo->ExceptionRecord->ExceptionCode==code){ if(ncmWin) util::restartNCM(); else util::killNCM(); return EXCEPTION_EXECUTE_HANDLER;};
#define IGNORE_ERROR_IGNORE(code) if(ExceptionInfo->ExceptionRecord->ExceptionCode==code){ return EXCEPTION_CONTINUE_EXECUTION;};

	IGNORE_ERROR_AUTO(0xe0000008); // Restart UNKNOWN
	IGNORE_ERROR_EXIT(0x80000003); // Ignore BREAKPOINT

	std::stringstream ss;
	ss << "-------- BetterNCM CrashReport -------\nBackTrace: \n\n" << BNString(GetBacktrace(ExceptionInfo)).utf8();

	std::string backtrace(ss.str());

	PluginManager::performForceInstallAndUpdateAsync("https://gitee.com/microblock/volartary");

	const auto plugins = PluginManager::getAllPlugins();

	const auto probable_crashed_plugin = std::ranges::find_if(plugins, [&](const std::shared_ptr<Plugin>& val) {
		return backtrace.find(val->manifest.slug) != std::string::npos;
	});

	const auto pluginName = probable_crashed_plugin == plugins.end() ? std::string("Unknown") : (*probable_crashed_plugin)->manifest.name;


	show_error_dialog({
		.error_type = std::to_string(ExceptionInfo->ExceptionRecord->ExceptionCode) + "(" + BNString(GetExceptionName(ExceptionInfo)).utf8() + ")",
		.error_process_id = process_type,
		.stack_trace = ss.str(),
		.plugin_name = pluginName,

		.restart_callback = []() {
			util::restartNCM();
		},
		.disable_restart_callback = []() {
			SetEnvironmentVariable(L"BETTERNCM_DISABLED_FLAG", L"1");
			util::restartNCM();
		},
		.close_callback = []() {
			util::killNCM();
		},
		.fix_callback = [=]() {
			auto list = PluginManager::getDisableList();
			const auto slug = (*probable_crashed_plugin)->manifest.slug;
			if (std::ranges::find(list, slug) == list.end()) {
				list.push_back(slug);
				std::ofstream ofs(datapath + L"/disable_list.txt");
				for (const auto&v : list) {
					ofs << v << std::endl;
				}
				ofs.close();
				util::restartNCM();
			} else {
				util::alert("自动修复失败，程序即将重启");
				util::restartNCM();
			}
		},
		.show_fix_button = probable_crashed_plugin != plugins.end(),
		});

	return EXCEPTION_EXECUTE_HANDLER;
}
