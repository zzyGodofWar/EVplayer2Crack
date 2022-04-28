#include <Windows.h>
#include <processthreadsapi.h>
#include "MinHook.h"

#ifdef _WIN64
#pragma comment(lib,"libMinHook.x64.lib")
#else
#pragma comment(lib,"libMinHook.x86.lib")
#endif // _WIN64


void InitApiHook();

HWND thisHwnd;

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)InitApiHook, 0, 0, 0);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

PVOID orgSetWindowDisplayAffinity;
PVOID orgOpenProcess;

typedef BOOL(WINAPI *lpSetWindowDisplayAffinity)(HWND, DWORD);

BOOL WINAPI mySetWindowDisplayAffinity(
	HWND  hWnd,
	DWORD dwAffinity
) {
	return ((lpSetWindowDisplayAffinity)orgSetWindowDisplayAffinity)(hWnd, 0);
}

HANDLE
WINAPI
myOpenProcess(
	_In_ DWORD dwDesiredAccess,
	_In_ BOOL bInheritHandle,
	_In_ DWORD dwProcessId
) {
	return (HANDLE)-1;
}



void InitApiHook() {
	if (MH_Initialize()) {
		MessageBoxA(NULL, "≥ı ºªØ ß∞‹", " ß∞‹", 0);
	}

	if (MH_CreateHook(SetWindowDisplayAffinity, mySetWindowDisplayAffinity, &orgSetWindowDisplayAffinity)) {
		MessageBoxA(NULL, "¥¥Ω®HOOK ß∞‹", " ß∞‹", 0);
	}
	MH_EnableHook(SetWindowDisplayAffinity);



	thisHwnd = FindWindowA("Qt5QWindowIcon", "EVPlayer2");
	SetWindowDisplayAffinity(thisHwnd, 0);
	MessageBoxA(NULL, "By Fizzy", "∆∆Ω‚≥…π¶", 0);

	if (MH_CreateHook(OpenProcess, myOpenProcess, &orgOpenProcess)) {
		MessageBoxA(NULL, "¥¥Ω®HOOK ß∞‹", " ß∞‹", 0);
	}
	MH_EnableHook(OpenProcess);

}



