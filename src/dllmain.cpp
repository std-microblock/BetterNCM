#include "pch.h"
#include "pystring/pystring.h"
#include "EasyCEFHooks.h"
#include <Windows.h>
#include "App.h"
#include "resource.h"
#include "utils/utils.h"
#include <stdlib.h>
#include <PluginManager.h>
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "Wininet.lib")

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
	MessageBox(nullptr, util::s2ws(text).c_str(), util::s2ws(title).c_str(), 0);
}


namespace AheadLib {
	HMODULE m_hModule = nullptr;
	DWORD m_dwReturn[5] = { 0 };
	App* app;

	inline BOOL WINAPI Load() {
		TCHAR tzPath[MAX_PATH];
		TCHAR tzTemp[MAX_PATH * 2];

		GetSystemDirectory(tzPath, MAX_PATH);
		lstrcat(tzPath, TEXT("\\msimg32"));
		m_hModule = LoadLibrary(tzPath);
		if (m_hModule == nullptr) {
			wsprintf(tzTemp, TEXT("�޷����� %s�������޷��������С�"), tzPath);
			MessageBox(nullptr, tzTemp, TEXT("AheadLib"), MB_ICONSTOP);
		}

		return (m_hModule != nullptr);
	}

	inline VOID WINAPI Free() {
		if (m_hModule) {
			FreeLibrary(m_hModule);
		}
	}

	FARPROC WINAPI GetAddress(PCSTR pszProcName) {
		FARPROC fpAddress;
		CHAR szProcName[16];
		TCHAR tzTemp[MAX_PATH];

		fpAddress = GetProcAddress(m_hModule, pszProcName);
		if (fpAddress == nullptr) {
			if (HIWORD(pszProcName) == 0) {
				pszProcName = szProcName;
			}

			wsprintf(tzTemp, TEXT("�޷��ҵ����� %hs�������޷��������С�"), pszProcName);
			MessageBox(nullptr, tzTemp, TEXT("AheadLib"), MB_ICONSTOP);
			ExitProcess(-2);
		}

		return fpAddress;
	}
}

using namespace AheadLib;
#pragma runtime_checks( "", off )
ALCDECL AheadLib_vSetDdrawflag(void) {
	__asm PUSH m_dwReturn[0 * TYPE long];
	__asm CALL DWORD PTR[TlsSetValue];

	GetAddress("vSetDdrawflag")();

	__asm PUSH EAX;
	__asm PUSH m_dwReturn[0 * TYPE long];
	__asm CALL DWORD PTR[TlsGetValue];
	__asm XCHG EAX, [ESP];
	__asm RET;
}

ALCDECL AheadLib_AlphaBlend(void) {
	__asm PUSH m_dwReturn[1 * TYPE long];
	__asm CALL DWORD PTR[TlsSetValue];

	GetAddress("AlphaBlend")();

	__asm PUSH EAX;
	__asm PUSH m_dwReturn[1 * TYPE long];
	__asm CALL DWORD PTR[TlsGetValue];
	__asm XCHG EAX, [ESP];
	__asm RET;
}

ALCDECL AheadLib_DllInitialize(void) {
	__asm PUSH m_dwReturn[2 * TYPE long];
	__asm CALL DWORD PTR[TlsSetValue];

	GetAddress("DllInitialize")();

	__asm PUSH EAX;
	__asm PUSH m_dwReturn[2 * TYPE long];
	__asm CALL DWORD PTR[TlsGetValue];
	__asm XCHG EAX, [ESP];
	__asm RET;
}

ALCDECL AheadLib_GradientFill(void) {
	__asm PUSH m_dwReturn[3 * TYPE long];
	__asm CALL DWORD PTR[TlsSetValue];

	GetAddress("GradientFill")();

	__asm PUSH EAX;
	__asm PUSH m_dwReturn[3 * TYPE long];
	__asm CALL DWORD PTR[TlsGetValue];
	__asm XCHG EAX, [ESP];
	__asm RET;
}

ALCDECL AheadLib_TransparentBlt(void) {
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
HMODULE g_hModule = nullptr;

extern BNString datapath;

NCMProcessType process_type = Undetected;
LONG WINAPI BNUnhandledExceptionFilter(EXCEPTION_POINTERS* ExceptionInfo);

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, PVOID pvReserved) {
	if (dwReason == DLL_PROCESS_ATTACH) {
		g_hModule = hModule;
		SetUnhandledExceptionFilter(BNUnhandledExceptionFilter);

		try {
			if (!getenv("BETTERNCM_DISABLED_FLAG")) {
				if (util::get_command_line().includes(L"--type=renderer"))process_type = Renderer;
				else if (util::get_command_line().includes(L"--type=gpu-process"))process_type = GpuProcess;
				else if (util::get_command_line().includes(L"--type=utility"))process_type = Utility;
				else process_type = Main;
				namespace fs = std::filesystem;

				// Pick data folder
				if (getenv("BETTERNCM_PROFILE")) {
					datapath = util::getEnvironment("BETTERNCM_PROFILE");
				}
				else {
					datapath = "C:\\betterncm"; // 不再向前兼容
				}

				if (process_type == Main) {
					AllocConsole();
					freopen("CONOUT$", "w", stdout);
					ShowWindow(GetConsoleWindow(), SW_HIDE);

					std::wcout << L"Data folder picked: " << datapath << "\n";

					if (static_cast<int>(fs::status((std::wstring)datapath).permissions()) & static_cast<int>(
						std::filesystem::perms::owner_write)) {
						// Create data folder
						fs::create_directories(datapath + L"/plugins");
						
						// PluginMarket
						if (!fs::exists(datapath + L"/plugins/PluginMarket.plugin")) {
							HRSRC myResource = ::FindResource(hModule, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);
							unsigned int myResourceSize = SizeofResource(hModule, myResource);
							HGLOBAL myResourceData = LoadResource(hModule, myResource);
							void* pMyBinaryData = LockResource(myResourceData);
							std::ofstream f(datapath + L"/plugins/PluginMarket.plugin", std::ios::out | std::ios::binary);
							f.write(static_cast<char*>(pMyBinaryData), myResourceSize);
							f.close();
						}

						// Inject NCM
						app = new App();
					}
					else {
						util::alert(L"BetterNCM访问数据目录失败！可能需要以管理员身份运行或更改数据目录。\n\nBetterNCM将不会运行");
					}
				}
				else if (process_type == Renderer) {
					EasyCEFHooks::InstallHooks();

					PluginManager::loadAll();
					SetUnhandledExceptionFilter(BNUnhandledExceptionFilter);
					for (auto& plugin : PluginManager::getAllPlugins()) {
						plugin->loadNativePluginDll(process_type);
					}
				}
				else {
					PluginManager::loadAll();
					SetUnhandledExceptionFilter(BNUnhandledExceptionFilter);
					for (auto& plugin : PluginManager::getAllPlugins()) {
						plugin->loadNativePluginDll(process_type);
					}
				}
			}

		}
		catch (std::exception& e) {
			util::alert("BetterNCM 崩溃了！\n\nBetterNCM 将不会运行\n网易云将有可能崩溃\n\n崩溃原因：" + std::string(e.what()));
		}


		DisableThreadLibraryCalls(hModule);

		for (INT i = 0; i < sizeof(m_dwReturn) / sizeof(DWORD); i++) {
			m_dwReturn[i] = TlsAlloc();
		}

		return Load();
	}
	if (dwReason == DLL_PROCESS_DETACH) {
		if (app)
			delete app;

		if (!getenv("BETTERNCM_DISABLED_FLAG")) {
			EasyCEFHooks::UninstallHook();
		}

		for (INT i = 0; i < sizeof(m_dwReturn) / sizeof(DWORD); i++) {
			TlsFree(m_dwReturn[i]);
		}

		Free();
	}
	return TRUE;
}
