
//
// created by AheadLib
// github:https://github.com/strivexjun/AheadLib-x86-x64
//
#include "pch.h"
#include <windows.h>
#include <Shlwapi.h>

#pragma comment( lib, "Shlwapi.lib")

#pragma comment(linker, "/EXPORT:vSetDdrawflag=_AheadLib_vSetDdrawflag,@1")
#pragma comment(linker, "/EXPORT:AlphaBlend=_AheadLib_AlphaBlend,@2")
#pragma comment(linker, "/EXPORT:DllInitialize=_AheadLib_DllInitialize,@3")
#pragma comment(linker, "/EXPORT:GradientFill=_AheadLib_GradientFill,@4")
#pragma comment(linker, "/EXPORT:TransparentBlt=_AheadLib_TransparentBlt,@5")


PVOID pfnAheadLib_vSetDdrawflag;
PVOID pfnAheadLib_AlphaBlend;
PVOID pfnAheadLib_DllInitialize;
PVOID pfnAheadLib_GradientFill;
PVOID pfnAheadLib_TransparentBlt;


static
HMODULE g_OldModule = NULL;

VOID WINAPI Free()
{
	if (g_OldModule)
	{
		FreeLibrary(g_OldModule);
	}
}


BOOL WINAPI Load()
{
	TCHAR tzPath[MAX_PATH];
	TCHAR tzTemp[MAX_PATH * 2];

	GetSystemDirectory(tzPath, MAX_PATH);

	lstrcat(tzPath, TEXT("\\msimg32.dll"));

	g_OldModule = LoadLibrary(tzPath);
	if (g_OldModule == NULL)
	{
		wsprintf(tzTemp, TEXT("无法找到模块 %s,程序无法正常运行"), tzPath);
		MessageBox(NULL, tzTemp, TEXT("AheadLib"), MB_ICONSTOP);
	}

	return (g_OldModule != NULL);

}


FARPROC WINAPI GetAddress(PCSTR pszProcName)
{
	FARPROC fpAddress;
	CHAR szProcName[64];
	TCHAR tzTemp[MAX_PATH];

	fpAddress = GetProcAddress(g_OldModule, pszProcName);
	if (fpAddress == NULL)
	{
		if (HIWORD(pszProcName) == 0)
		{
			wsprintfA(szProcName, "#%d", pszProcName);
			pszProcName = szProcName;
		}

		wsprintf(tzTemp, TEXT("无法找到函数 %hs,程序无法正常运行"), pszProcName);
		MessageBox(NULL, tzTemp, TEXT("AheadLib"), MB_ICONSTOP);
		ExitProcess(-2);
	}
	return fpAddress;
}

BOOL WINAPI Init()
{
	pfnAheadLib_vSetDdrawflag = GetAddress("vSetDdrawflag");
	pfnAheadLib_AlphaBlend = GetAddress("AlphaBlend");
	pfnAheadLib_DllInitialize = GetAddress("DllInitialize");
	pfnAheadLib_GradientFill = GetAddress("GradientFill");
	pfnAheadLib_TransparentBlt = GetAddress("TransparentBlt");
	return TRUE;
}	

void bncmMain();

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, PVOID pvReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);

		if (Load() && Init())
		{
			TCHAR szAppName[MAX_PATH] = TEXT("cloudmusic.exe");//请修改宿主进程名
			TCHAR szCurName[MAX_PATH];

			GetModuleFileName(NULL, szCurName, MAX_PATH);
			PathStripPath(szCurName);

			if (StrCmpI(szCurName, szAppName) == 0)
			{
				bncmMain();
			}
		}
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		Free();
	}

	return TRUE;
}

EXTERN_C __declspec(naked) void __cdecl AheadLib_vSetDdrawflag(void)
{
	__asm jmp pfnAheadLib_vSetDdrawflag;
}

EXTERN_C __declspec(naked) void __cdecl AheadLib_AlphaBlend(void)
{
	__asm jmp pfnAheadLib_AlphaBlend;
}

EXTERN_C __declspec(naked) void __cdecl AheadLib_DllInitialize(void)
{
	__asm jmp pfnAheadLib_DllInitialize;
}

EXTERN_C __declspec(naked) void __cdecl AheadLib_GradientFill(void)
{
	__asm jmp pfnAheadLib_GradientFill;
}

EXTERN_C __declspec(naked) void __cdecl AheadLib_TransparentBlt(void)
{
	__asm jmp pfnAheadLib_TransparentBlt;
}

