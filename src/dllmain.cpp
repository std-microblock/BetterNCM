#include "pch.h"
#include "pystring/pystring.h"
#include "EasyCEFHooks.h"
#include <Windows.h>
#include "App.h"
#include "resource.h"
#include "utils/utils.h"
#include <stdlib.h>
#include <NativePlugin.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")


#pragma comment(linker, "/EXPORT:vSetDdrawflag=_AheadLib_vSetDdrawflag,@1")
#pragma comment(linker, "/EXPORT:AlphaBlend=_AheadLib_AlphaBlend,@2") 
#pragma comment(linker, "/EXPORT:DllInitialize=_AheadLib_DllInitialize,@3")
#pragma comment(linker, "/EXPORT:GradientFill=_AheadLib_GradientFill,@4")
#pragma comment(linker, "/EXPORT:TransparentBlt=_AheadLib_TransparentBlt,@5") 

#define EXTERNC extern "C"
#define NAKED __declspec(naked)
#define EXPORT __declspec(dllexport)

#define ALCPP EXPORT NAKED 
#define ALSTD EXTERNC EXPORT NAKED void __stdcall
#define ALCFAST EXTERNC EXPORT NAKED void __fastcall
#define ALCDECL EXTERNC NAKED void __cdecl

std::string script;


void message(const std::string& title, const std::string& text) {
	MessageBox(NULL, util::s2ws(text).c_str(), util::s2ws(title).c_str(), 0);
}


namespace AheadLib
{
	HMODULE m_hModule = NULL;
	DWORD m_dwReturn[5] = { 0 };
	App* app;

	inline BOOL WINAPI Load()
	{
		TCHAR tzPath[MAX_PATH];
		TCHAR tzTemp[MAX_PATH * 2];

		GetSystemDirectory(tzPath, MAX_PATH);
		lstrcat(tzPath, TEXT("\\msimg32"));
		m_hModule = LoadLibrary(tzPath);
		if (m_hModule == NULL)
		{
			wsprintf(tzTemp, TEXT("�޷����� %s�������޷��������С�"), tzPath);
			MessageBox(NULL, tzTemp, TEXT("AheadLib"), MB_ICONSTOP);
		}

		return (m_hModule != NULL);
	}

	inline VOID WINAPI Free()
	{
		if (m_hModule)
		{
			FreeLibrary(m_hModule);
		}
	}

	FARPROC WINAPI GetAddress(PCSTR pszProcName)
	{
		FARPROC fpAddress;
		CHAR szProcName[16];
		TCHAR tzTemp[MAX_PATH];

		fpAddress = GetProcAddress(m_hModule, pszProcName);
		if (fpAddress == NULL)
		{
			if (HIWORD(pszProcName) == 0)
			{
				pszProcName = szProcName;
			}

			wsprintf(tzTemp, TEXT("�޷��ҵ����� %hs�������޷��������С�"), pszProcName);
			MessageBox(NULL, tzTemp, TEXT("AheadLib"), MB_ICONSTOP);
			ExitProcess(-2);
		}

		return fpAddress;
	}
}
using namespace AheadLib;
#pragma runtime_checks( "", off )
ALCDECL AheadLib_vSetDdrawflag(void)
{
	__asm PUSH m_dwReturn[0 * TYPE long];
	__asm CALL DWORD PTR[TlsSetValue];

	GetAddress("vSetDdrawflag")();

	__asm PUSH EAX;
	__asm PUSH m_dwReturn[0 * TYPE long];
	__asm CALL DWORD PTR[TlsGetValue];
	__asm XCHG EAX, [ESP];
	__asm RET;
}

ALCDECL AheadLib_AlphaBlend(void)
{
	__asm PUSH m_dwReturn[1 * TYPE long];
	__asm CALL DWORD PTR[TlsSetValue];

	GetAddress("AlphaBlend")();

	__asm PUSH EAX;
	__asm PUSH m_dwReturn[1 * TYPE long];
	__asm CALL DWORD PTR[TlsGetValue];
	__asm XCHG EAX, [ESP];
	__asm RET;
}

ALCDECL AheadLib_DllInitialize(void)
{
	__asm PUSH m_dwReturn[2 * TYPE long];
	__asm CALL DWORD PTR[TlsSetValue];

	GetAddress("DllInitialize")();

	__asm PUSH EAX;
	__asm PUSH m_dwReturn[2 * TYPE long];
	__asm CALL DWORD PTR[TlsGetValue];
	__asm XCHG EAX, [ESP];
	__asm RET;
}

ALCDECL AheadLib_GradientFill(void)
{
	__asm PUSH m_dwReturn[3 * TYPE long];
	__asm CALL DWORD PTR[TlsSetValue];

	GetAddress("GradientFill")();

	__asm PUSH EAX;
	__asm PUSH m_dwReturn[3 * TYPE long];
	__asm CALL DWORD PTR[TlsGetValue];
	__asm XCHG EAX, [ESP];
	__asm RET;
}

ALCDECL AheadLib_TransparentBlt(void)
{
	__asm PUSH m_dwReturn[4 * TYPE long];
	__asm CALL DWORD PTR[TlsSetValue];

	GetAddress("TransparentBlt")();

	__asm PUSH EAX;
	__asm PUSH m_dwReturn[4 * TYPE long];
	__asm CALL DWORD PTR[TlsGetValue];
	__asm XCHG EAX, [ESP];
	__asm RET;
}
#pragma runtime_checks( "", restore )
HMODULE  g_hModule = nullptr;

extern BNString datapath;

NCMProcessType process_type = NCMProcessType::Undetected;


std::string GetBacktrace(EXCEPTION_POINTERS* ExceptionInfo)
{
	std::string info;
	char symbol_mem[sizeof(IMAGEHLP_SYMBOL64) + 256];
	IMAGEHLP_SYMBOL64* symbol = (IMAGEHLP_SYMBOL64*)symbol_mem;


	// Get the context record from the exception information
	CONTEXT* contextRecord = ExceptionInfo->ContextRecord;

	// Initialize the symbol handler
	SymInitialize(GetCurrentProcess(), NULL, TRUE);

	// Initialize the stack frame
	STACKFRAME64 stackFrame = { 0 };
	stackFrame.AddrPC.Offset = contextRecord->Eip;
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Offset = contextRecord->Ebp;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
	stackFrame.AddrStack.Offset = contextRecord->Esp;
	stackFrame.AddrStack.Mode = AddrModeFlat;

	DWORD64 displacement = 0;
	// Walk the call stack and append each frame to the string
	while (StackWalk64(IMAGE_FILE_MACHINE_I386, GetCurrentProcess(), GetCurrentThread(), &stackFrame, contextRecord, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
	{
		// Get the module name and offset for this frame
		DWORD64 moduleBase = SymGetModuleBase64(GetCurrentProcess(), stackFrame.AddrPC.Offset);
		char moduleName[MAX_PATH];
		GetModuleFileNameA((HMODULE)moduleBase, moduleName, MAX_PATH);
		DWORD64 offset = stackFrame.AddrPC.Offset - moduleBase;

		symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
		symbol->MaxNameLength = 255;

		char name[256] = {};
		SymGetSymFromAddr64(GetCurrentProcess(), (ULONG64)stackFrame.AddrPC.Offset, &displacement, symbol);
		UnDecorateSymbolName(symbol->Name, (PSTR)name, 256, UNDNAME_COMPLETE);

		// Append the frame to the string
		char frameInfo[1024];
		sprintf_s(frameInfo, "%s + %X", moduleName, offset);
		info += frameInfo + std::string("(") + name + std::string(")\n");
	}

	// Cleanup the symbol handler
	SymCleanup(GetCurrentProcess());

	// Return the backtrace string
	return info;
}

std::string GetExceptionName(EXCEPTION_POINTERS* ExceptionInfo)
{
	std::string exceptionName;
	switch (ExceptionInfo->ExceptionRecord->ExceptionCode)
	{
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

std::wstring PrintExceptionInfo(EXCEPTION_POINTERS* ExceptionInfo)
{
	std::wostringstream Stream;

	EXCEPTION_RECORD* ExceptionRecord = ExceptionInfo->ExceptionRecord;
	CONTEXT* ContextRecord = ExceptionInfo->ContextRecord;

	Stream << L"Exception code: 0x" << std::hex << ExceptionRecord->ExceptionCode << "(" << BNString(GetExceptionName(ExceptionInfo)) << ")" << std::endl;
	Stream << L"Exception flags: 0x" << std::hex << ExceptionRecord->ExceptionFlags << std::endl;
	Stream << L"Exception address: 0x" << std::hex << ExceptionRecord->ExceptionAddress << std::endl;

	Stream << L"Context record:" << std::endl;
	Stream << L"  EIP: 0x" << std::hex << ContextRecord->Eip << std::endl;
	Stream << L"  ESP: 0x" << std::hex << ContextRecord->Esp << std::endl;
	Stream << L"  EBP: 0x" << std::hex << ContextRecord->Ebp << std::endl;

	Stream << L"Backtrace:\n  See console for detail" << std::endl;

	return Stream.str();
}

LONG WINAPI BNUnhandledExceptionFilter(EXCEPTION_POINTERS* ExceptionInfo)
{
	HWND ncmWin = FindWindow(L"OrpheusBrowserHost", NULL);

#define IGNORE_ERROR_EXIT(code) if(ExceptionInfo->ExceptionRecord->ExceptionCode==code){ util::killNCM(); return EXCEPTION_EXECUTE_HANDLER;}
#define IGNORE_ERROR_RESTART(code) if(ExceptionInfo->ExceptionRecord->ExceptionCode==code){ util::restartNCM(); return EXCEPTION_EXECUTE_HANDLER; }
#define IGNORE_ERROR_PASS_TO_NCM(code) if(ExceptionInfo->ExceptionRecord->ExceptionCode==code){ return EXCEPTION_CONTINUE_SEARCH; }
#define IGNORE_ERROR_AUTO(code) if(ExceptionInfo->ExceptionRecord->ExceptionCode==code){ if(ncmWin) util::restartNCM(); else util::killNCM(); return EXCEPTION_EXECUTE_HANDLER;};
#define IGNORE_ERROR_IGNORE(code) if(ExceptionInfo->ExceptionRecord->ExceptionCode==code){ return EXCEPTION_CONTINUE_EXECUTION;};

	IGNORE_ERROR_IGNORE(0xe0000008); // Ignore UNKNOWN
	IGNORE_ERROR_IGNORE(0x80000003); // Ignore BREAKPOINT

	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	ShowWindow(GetConsoleWindow(), SW_SHOW);

	for (int x = 0; x < 10; x++)std::cout << "\n";
	std::cout << "-------- BetterNCM CrashReport -------\nBackTrace: \n\n" << GetBacktrace(ExceptionInfo) << "\n\n--------------------------------------\n\n\n\n";

	int result = MessageBoxW(ncmWin,
		(L"很抱歉，网易云音乐崩溃了！\n\n" +
			PrintExceptionInfo(ExceptionInfo) +
			L"In Process: " + BNString(std::format("{:#x}", (int)process_type)) + L"\n" +
			L"\n这有可能是由于插件引起的崩溃，要重启网易云音乐吗？\n\n点击 中止 以直接结束网易云\n点击 重试 以直接重启网易云\n点击 忽略 以禁用插件并重启网易云").c_str(),
		L"BetterNCM 网易云音乐崩溃", MB_ABORTRETRYIGNORE | MB_ICONERROR);

	if (result == IDABORT)
	{
		util::killNCM();
	}
	if (result == IDRETRY)
	{
		util::restartNCM();
	}
	if (result == IDIGNORE)
	{
		SetEnvironmentVariable(L"BETTERNCM_DISABLED_FLAG", L"1");
		util::restartNCM();
	}
	return EXCEPTION_EXECUTE_HANDLER;
}


BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, PVOID pvReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		g_hModule = hModule;
		SetUnhandledExceptionFilter(BNUnhandledExceptionFilter);



		if (!getenv("BETTERNCM_DISABLED_FLAG")) {
			if (util::get_command_line().includes(L"--type=renderer"))process_type = NCMProcessType::Renderer;
			else if (util::get_command_line().includes(L"--type=gpu-process"))process_type = NCMProcessType::GpuProcess;
			else if (util::get_command_line().includes(L"--type=utility"))process_type = NCMProcessType::Utility;
			else process_type = NCMProcessType::Main;
			namespace fs = std::filesystem;

			// Pick data folder
			if (getenv("BETTERNCM_PROFILE")) {
				datapath = util::getEnvironment("BETTERNCM_PROFILE");
			}
			else {
				datapath = "C:\\betterncm"; // 不再向前兼容
			}


			if (process_type == NCMProcessType::Main) {
				AllocConsole();
				freopen("CONOUT$", "w", stdout);
				ShowWindow(GetConsoleWindow(), SW_HIDE);

				std::wcout << L"Data folder picked: " << datapath << "\n";

				if ((int)fs::status((std::wstring)datapath).permissions() & (int)std::filesystem::perms::owner_write) {
					// Create data folder
					fs::create_directories(datapath + L"/plugins");
					// PluginMarket
					HRSRC myResource = ::FindResource(hModule, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);
					unsigned int myResourceSize = ::SizeofResource(hModule, myResource);
					HGLOBAL myResourceData = ::LoadResource(hModule, myResource);
					void* pMyBinaryData = ::LockResource(myResourceData);
					std::ofstream f(datapath + L"/plugins/PluginMarket.plugin", std::ios::out | std::ios::binary);
					f.write((char*)pMyBinaryData, myResourceSize);
					f.close();

					// Inject NCM
					app = new App();
				}
				else {
					util::alert(L"BetterNCM访问数据目录失败！可能需要以管理员身份运行或更改数据目录。\n\nBetterNCM将不会运行");
				}
			}
			else if (process_type == NCMProcessType::Renderer) {
				EasyCEFHooks::InstallHooks();

				PluginsLoader::loadAll();

				for (auto& plugin : PluginsLoader::plugins) {
					plugin.loadNativePluginDll(process_type);
				}
			}
			else {
				PluginsLoader::loadAll();

				for (auto& plugin : PluginsLoader::plugins) {
					plugin.loadNativePluginDll(process_type);
				}
			}

		}







		DisableThreadLibraryCalls(hModule);

		for (INT i = 0; i < sizeof(m_dwReturn) / sizeof(DWORD); i++)
		{
			m_dwReturn[i] = TlsAlloc();
		}

		return Load();
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		if (app)
			delete app;

		if (!getenv("BETTERNCM_DISABLED_FLAG")) {
			EasyCEFHooks::UninstallHook();
		}

		for (INT i = 0; i < sizeof(m_dwReturn) / sizeof(DWORD); i++)
		{
			TlsFree(m_dwReturn[i]);
		}

		Free();
	}
	return TRUE;
}
